#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#include "hrmcli/terminal.h"

static struct termios original_termios;

static int terminal_active = 0;
static int atexit_registered = 0;

static int pushed_byte = -1;

static volatile sig_atomic_t resize_pending = 0;
static volatile sig_atomic_t termination_pending = 0;

static struct sigaction original_sigwinch_action;
static struct sigaction original_sigterm_action;
static struct sigaction original_sighup_action;
static struct sigaction original_sigint_action;
static struct sigaction original_sigquit_action;

static int sigwinch_installed = 0;
static int sigterm_installed = 0;
static int sighup_installed = 0;
static int sigint_installed = 0;
static int sigquit_installed = 0;


static void write_all(const char *data, size_t length)
{
    while (length > 0) {
        ssize_t written;

        written = write(
            STDOUT_FILENO,
            data,
            length
        );

        if (written < 0) {
            if (errno == EINTR) {
                continue;
            }

            return;
        }

        data += written;
        length -= (size_t)written;
    }
}


static void handle_sigwinch(int signal_number)
{
    (void)signal_number;

    resize_pending = 1;
}


static void handle_termination_signal(int signal_number)
{
    (void)signal_number;

    termination_pending = 1;
}


static int install_signal_handler(
    int signal_number,
    void (*handler)(int),
    struct sigaction *original_action
)
{
    struct sigaction action;

    memset(&action, 0, sizeof(action));

    action.sa_handler = handler;

    sigemptyset(&action.sa_mask);

    action.sa_flags = 0;

    if (sigaction(
            signal_number,
            &action,
            original_action
        ) == -1) {

        return -1;
    }

    return 0;
}


static void restore_signal_handlers(void)
{
    if (sigwinch_installed) {
        sigaction(
            SIGWINCH,
            &original_sigwinch_action,
            NULL
        );

        sigwinch_installed = 0;
    }

    if (sigterm_installed) {
        sigaction(
            SIGTERM,
            &original_sigterm_action,
            NULL
        );

        sigterm_installed = 0;
    }

    if (sighup_installed) {
        sigaction(
            SIGHUP,
            &original_sighup_action,
            NULL
        );

        sighup_installed = 0;
    }

    if (sigint_installed) {
        sigaction(
            SIGINT,
            &original_sigint_action,
            NULL
        );

        sigint_installed = 0;
    }

    if (sigquit_installed) {
        sigaction(
            SIGQUIT,
            &original_sigquit_action,
            NULL
        );

        sigquit_installed = 0;
    }
}


int terminal_init(void)
{
    struct termios raw;

    if (
        !isatty(STDIN_FILENO) ||
        !isatty(STDOUT_FILENO)
    ) {
        return -1;
    }

    if (
        tcgetattr(
            STDIN_FILENO,
            &original_termios
        ) == -1
    ) {
        return -1;
    }

    raw = original_termios;

    /*
     * Configure a raw-style terminal mode.
     *
     * ISIG is disabled so Ctrl+C arrives as byte 3 and
     * can be handled by HRMCLi itself.
     */
    raw.c_iflag &= ~(
        BRKINT |
        ICRNL |
        INPCK |
        ISTRIP |
        IXON
    );

    raw.c_oflag &= ~(OPOST);

    raw.c_cflag |= CS8;

    raw.c_lflag &= ~(
        ECHO |
        ICANON |
        IEXTEN |
        ISIG
    );

    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (
        tcsetattr(
            STDIN_FILENO,
            TCSAFLUSH,
            &raw
        ) == -1
    ) {
        return -1;
    }

    /*
     * Install resize handler.
     */
    if (
        install_signal_handler(
            SIGWINCH,
            handle_sigwinch,
            &original_sigwinch_action
        ) == -1
    ) {
        goto failure;
    }

    sigwinch_installed = 1;

    /*
     * Install clean-termination handlers.
     */
    if (
        install_signal_handler(
            SIGTERM,
            handle_termination_signal,
            &original_sigterm_action
        ) == -1
    ) {
        goto failure;
    }

    sigterm_installed = 1;

    if (
        install_signal_handler(
            SIGHUP,
            handle_termination_signal,
            &original_sighup_action
        ) == -1
    ) {
        goto failure;
    }

    sighup_installed = 1;

    if (
        install_signal_handler(
            SIGINT,
            handle_termination_signal,
            &original_sigint_action
        ) == -1
    ) {
        goto failure;
    }

    sigint_installed = 1;

    if (
        install_signal_handler(
            SIGQUIT,
            handle_termination_signal,
            &original_sigquit_action
        ) == -1
    ) {
        goto failure;
    }

    sigquit_installed = 1;

    terminal_active = 1;

    /*
     * atexit gives us an additional safety net for normal
     * process termination paths.
     */
    if (!atexit_registered) {
        if (atexit(terminal_shutdown) != 0) {
            goto failure;
        }

        atexit_registered = 1;
    }

    terminal_hide_cursor();

    return 0;


failure:

    restore_signal_handlers();

    tcsetattr(
        STDIN_FILENO,
        TCSAFLUSH,
        &original_termios
    );

    terminal_active = 0;

    return -1;
}

void terminal_set_reverse(int enabled)
{
    if (enabled) {
        terminal_write("\x1b[7m");
    } else {
        terminal_write("\x1b[27m");
    }
}

void terminal_shutdown(void)
{
    if (!terminal_active) {
        return;
    }

    /*
     * Restore visual state first.
     */
    terminal_write("\x1b[0m");
    terminal_show_cursor();

    /*
     * Restore the user's original terminal configuration.
     */
    tcsetattr(
        STDIN_FILENO,
        TCSAFLUSH,
        &original_termios
    );

    restore_signal_handlers();

    resize_pending = 0;
    termination_pending = 0;
    pushed_byte = -1;

    terminal_active = 0;
}


int terminal_get_size(TerminalSize *size)
{
    struct winsize ws;

    if (size == NULL) {
        return -1;
    }

    if (
        ioctl(
            STDOUT_FILENO,
            TIOCGWINSZ,
            &ws
        ) == -1
    ) {
        return -1;
    }

    if (
        ws.ws_row == 0 ||
        ws.ws_col == 0
    ) {
        return -1;
    }

    size->rows = ws.ws_row;
    size->cols = ws.ws_col;

    return 0;
}


void terminal_clear(void)
{
    terminal_write("\x1b[2J");
    terminal_write("\x1b[H");
}


void terminal_move_cursor(int row, int col)
{
    char sequence[32];
    int length;

    if (row < 1) {
        row = 1;
    }

    if (col < 1) {
        col = 1;
    }

    length = snprintf(
        sequence,
        sizeof(sequence),
        "\x1b[%d;%dH",
        row,
        col
    );

    if (length > 0) {
        write_all(
            sequence,
            (size_t)length
        );
    }
}


void terminal_hide_cursor(void)
{
    terminal_write("\x1b[?25l");
}


void terminal_show_cursor(void)
{
    terminal_write("\x1b[?25h");
}


void terminal_write(const char *text)
{
    if (text == NULL) {
        return;
    }

    write_all(
        text,
        strlen(text)
    );
}


static int read_byte_blocking(unsigned char *ch)
{
    ssize_t result;

    if (pushed_byte >= 0) {
        *ch = (unsigned char)pushed_byte;

        pushed_byte = -1;

        return 1;
    }

    while (1) {
        result = read(
            STDIN_FILENO,
            ch,
            1
        );

        if (result == 1) {
            return 1;
        }

        if (
            result < 0 &&
            errno == EINTR
        ) {
            /*
             * A signal interrupted the blocking read.
             * Let terminal_read_key() determine which
             * logical event occurred.
             */
            if (
                resize_pending ||
                termination_pending
            ) {
                return 0;
            }

            continue;
        }

        return -1;
    }
}


static int read_byte_timeout(
    unsigned char *ch,
    int timeout_ms
)
{
    struct pollfd fd;

    int result;

    if (pushed_byte >= 0) {
        *ch = (unsigned char)pushed_byte;

        pushed_byte = -1;

        return 1;
    }

    fd.fd = STDIN_FILENO;
    fd.events = POLLIN;
    fd.revents = 0;

    while (1) {
        result = poll(
            &fd,
            1,
            timeout_ms
        );

        if (result > 0) {
            break;
        }

        if (result == 0) {
            return 0;
        }

        if (
            result < 0 &&
            errno == EINTR
        ) {
            if (
                resize_pending ||
                termination_pending
            ) {
                return 0;
            }

            continue;
        }

        return -1;
    }

    if (!(fd.revents & POLLIN)) {
        return -1;
    }

    return read_byte_blocking(ch);
}


static int get_pending_event(void)
{
    /*
     * Termination takes priority over resize.
     */
    if (termination_pending) {
        return TERMINAL_KEY_TERMINATE;
    }

    if (resize_pending) {
        resize_pending = 0;

        return TERMINAL_KEY_RESIZE;
    }

    return 0;
}


int terminal_read_key(void)
{
    unsigned char first;
    unsigned char second;
    unsigned char third;
    unsigned char fourth;

    int result;
    int event;

    event = get_pending_event();

    if (event != 0) {
        return event;
    }

    result = read_byte_blocking(&first);

    if (result == 0) {
        event = get_pending_event();

        if (event != 0) {
            return event;
        }

        return -1;
    }

    if (result != 1) {
        return -1;
    }

    switch (first) {
    case '\r':
    case '\n':
        return TERMINAL_KEY_ENTER;

    case 127:
    case 8:
        return TERMINAL_KEY_BACKSPACE;

    case '\t':
        return TERMINAL_KEY_TAB;

    case 3:
        return TERMINAL_KEY_CTRL_C;

    default:
        break;
    }

    /*
     * Anything other than ESC is an ordinary character.
     */
    if (first != 27) {
        return (int)first;
    }

    /*
     * ESC can either be the Escape key itself or the
     * beginning of an ANSI escape sequence.
     */
    result = read_byte_timeout(
        &second,
        30
    );

    if (result == 0) {
        event = get_pending_event();

        if (event != 0) {
            return event;
        }

        return TERMINAL_KEY_ESCAPE;
    }

    if (result < 0) {
        return -1;
    }

    /*
     * CSI sequence: ESC [
     */
    if (second == '[') {
        result = read_byte_timeout(
            &third,
            30
        );

        if (result == 0) {
            event = get_pending_event();

            if (event != 0) {
                return event;
            }

            return TERMINAL_KEY_ESCAPE;
        }

        if (result < 0) {
            return -1;
        }

        switch (third) {
        case 'A':
            return TERMINAL_KEY_UP;

        case 'B':
            return TERMINAL_KEY_DOWN;

        case 'C':
            return TERMINAL_KEY_RIGHT;

        case 'D':
            return TERMINAL_KEY_LEFT;

        case 'H':
            return TERMINAL_KEY_HOME;

        case 'F':
            return TERMINAL_KEY_END;

        case '1':
        case '3':
        case '4':
        case '7':
        case '8':
            result = read_byte_timeout(
                &fourth,
                30
            );

            if (result == 0) {
                event = get_pending_event();

                if (event != 0) {
                    return event;
                }

                return TERMINAL_KEY_ESCAPE;
            }

            if (
                result < 0 ||
                fourth != '~'
            ) {
                return TERMINAL_KEY_ESCAPE;
            }

            switch (third) {
            case '1':
            case '7':
                return TERMINAL_KEY_HOME;

            case '3':
                return TERMINAL_KEY_DELETE;

            case '4':
            case '8':
                return TERMINAL_KEY_END;

            default:
                return TERMINAL_KEY_ESCAPE;
            }

        default:
            return TERMINAL_KEY_ESCAPE;
        }
    }

    /*
     * SS3 sequence: ESC O
     */
    if (second == 'O') {
        result = read_byte_timeout(
            &third,
            30
        );

        if (result == 0) {
            event = get_pending_event();

            if (event != 0) {
                return event;
            }

            return TERMINAL_KEY_ESCAPE;
        }

        if (result < 0) {
            return -1;
        }

        switch (third) {
        case 'A':
            return TERMINAL_KEY_UP;

        case 'B':
            return TERMINAL_KEY_DOWN;

        case 'C':
            return TERMINAL_KEY_RIGHT;

        case 'D':
            return TERMINAL_KEY_LEFT;

        case 'H':
            return TERMINAL_KEY_HOME;

        case 'F':
            return TERMINAL_KEY_END;

        default:
            return TERMINAL_KEY_ESCAPE;
        }
    }

    /*
     * ESC followed by an unrelated character.
     * Preserve that character for the next read.
     */
    pushed_byte = second;

    return TERMINAL_KEY_ESCAPE;
}
