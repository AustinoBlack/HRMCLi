#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <poll.h>

#include "hrmcli/terminal.h"

static struct termios original_termios;
static int terminal_active = 0;

static void write_all(const char *data, size_t length)
{
    while (length > 0) {
        ssize_t written = write(STDOUT_FILENO, data, length);

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

int terminal_init(void)
{
    struct termios raw;

    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) {
        return -1;
    }

    if (tcgetattr(STDIN_FILENO, &original_termios) == -1) {
        return -1;
    }

    raw = original_termios;

    /*
     * Put the terminal into a simple raw-style mode.
     *
     * We intentionally handle Ctrl+C ourselves for now
     * so HRMCLi can restore the terminal before exiting.
     */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= CS8;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        return -1;
    }

    terminal_active = 1;

    terminal_hide_cursor();

    return 0;
}

void terminal_shutdown(void)
{
    if (!terminal_active) {
        return;
    }

    /*
     * Reset visual attributes and make the cursor visible
     * before returning control to the shell.
     */
    terminal_write("\x1b[0m");
    terminal_show_cursor();

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);

    terminal_active = 0;
}

int terminal_get_size(TerminalSize *size)
{
    struct winsize ws;

    if (size == NULL) {
        return -1;
    }

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) {
        return -1;
    }

    if (ws.ws_row == 0 || ws.ws_col == 0) {
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

    if (row < 1) {
        row = 1;
    }

    if (col < 1) {
        col = 1;
    }

    int length = snprintf(
        sequence,
        sizeof(sequence),
        "\x1b[%d;%dH",
        row,
        col
    );

    if (length > 0) {
        write_all(sequence, (size_t)length);
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

    write_all(text, strlen(text));
}

int terminal_read_byte(char *ch)
{
    ssize_t result;

    if (ch == NULL) {
        return -1;
    }

    do {
        result = read(STDIN_FILENO, ch, 1);
    } while (result < 0 && errno == EINTR);

    if (result != 1) {
        return -1;
    }

    return 0;
}

static int pushed_byte = -1;

static int read_byte_blocking(unsigned char *ch)
{
    ssize_t result;

    if (pushed_byte >= 0) {
        *ch = (unsigned char)pushed_byte;
        pushed_byte = -1;
        return 1;
    }

    do {
        result = read(STDIN_FILENO, ch, 1);
    } while (result < 0 && errno == EINTR);

    if (result == 1) {
        return 1;
    }

    return -1;
}

static int read_byte_timeout(unsigned char *ch, int timeout_ms)
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

    do {
        result = poll(&fd, 1, timeout_ms);
    } while (result < 0 && errno == EINTR);

    if (result == 0) {
        return 0;
    }

    if (result < 0) {
        return -1;
    }

    if (!(fd.revents & POLLIN)) {
        return -1;
    }

    return read_byte_blocking(ch);
}

int terminal_read_key(void)
{
    unsigned char first;
    unsigned char second;
    unsigned char third;
    unsigned char fourth;

    int result;

    if (read_byte_blocking(&first) != 1) {
        return -1;
    }

    /*
     * Handle ordinary single-byte keys first.
     */
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
     * Anything other than ESC is simply returned as
     * its character value.
     */
    if (first != 27) {
        return (int)first;
    }

    /*
     * ESC may either be the Escape key by itself or the
     * beginning of an ANSI escape sequence.
     *
     * Wait briefly to see if more bytes follow.
     */
    result = read_byte_timeout(&second, 30);

    if (result == 0) {
        return TERMINAL_KEY_ESCAPE;
    }

    if (result < 0) {
        return -1;
    }

    /*
     * CSI sequences normally begin with ESC [
     */
    if (second == '[') {
        result = read_byte_timeout(&third, 30);

        if (result != 1) {
            return TERMINAL_KEY_ESCAPE;
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

        /*
         * Some keys use sequences such as:
         *
         * ESC [ 3 ~
         */
        case '1':
        case '3':
        case '4':
        case '7':
        case '8':
            result = read_byte_timeout(&fourth, 30);

            if (result != 1 || fourth != '~') {
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
     * Some terminals use ESC O instead of ESC [
     * for certain navigation keys.
     */
    if (second == 'O') {
        result = read_byte_timeout(&third, 30);

        if (result != 1) {
            return TERMINAL_KEY_ESCAPE;
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
     * ESC was pressed immediately before an unrelated
     * character. Preserve that character for the next call.
     */
    pushed_byte = second;

    return TERMINAL_KEY_ESCAPE;
}
