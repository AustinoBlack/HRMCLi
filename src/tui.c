#include <stdio.h>
#include <string.h>

#include "hrmcli/terminal.h"
#include "hrmcli/tui.h"

static void draw_centered(
    int row,
    const char *text,
    int terminal_width
)
{
    int length;
    int col;

    length = (int)strlen(text);

    col = ((terminal_width - length) / 2) + 1;

    if (col < 1) {
        col = 1;
    }

    terminal_move_cursor(row, col);
    terminal_write(text);
}

static void draw_screen(const char *last_event)
{
    TerminalSize size;
    char dimensions[64];

    if (terminal_get_size(&size) != 0) {
        return;
    }

    snprintf(
        dimensions,
        sizeof(dimensions),
        "Terminal size: %d rows x %d columns",
        size.rows,
        size.cols
    );

    terminal_clear();

    draw_centered(
        size.rows / 2 - 3,
        "HRMCLi",
        size.cols
    );

    draw_centered(
        size.rows / 2 - 1,
        dimensions,
        size.cols
    );

    draw_centered(
        size.rows / 2 + 1,
        last_event,
        size.cols
    );

    draw_centered(
        size.rows / 2 + 3,
        "Resize the terminal to test SIGWINCH",
        size.cols
    );

    draw_centered(
        size.rows / 2 + 5,
        "Press q or Ctrl+C to quit",
        size.cols
    );
}

static void describe_key(
    int key,
    char *buffer,
    size_t buffer_size
)
{
    switch (key) {
    case TERMINAL_KEY_UP:
        snprintf(buffer, buffer_size, "Key: Up");
        break;

    case TERMINAL_KEY_DOWN:
        snprintf(buffer, buffer_size, "Key: Down");
        break;

    case TERMINAL_KEY_LEFT:
        snprintf(buffer, buffer_size, "Key: Left");
        break;

    case TERMINAL_KEY_RIGHT:
        snprintf(buffer, buffer_size, "Key: Right");
        break;

    case TERMINAL_KEY_ENTER:
        snprintf(buffer, buffer_size, "Key: Enter");
        break;

    case TERMINAL_KEY_ESCAPE:
        snprintf(buffer, buffer_size, "Key: Escape");
        break;

    case TERMINAL_KEY_BACKSPACE:
        snprintf(buffer, buffer_size, "Key: Backspace");
        break;

    case TERMINAL_KEY_TAB:
        snprintf(buffer, buffer_size, "Key: Tab");
        break;

    case TERMINAL_KEY_DELETE:
        snprintf(buffer, buffer_size, "Key: Delete");
        break;

    case TERMINAL_KEY_HOME:
        snprintf(buffer, buffer_size, "Key: Home");
        break;

    case TERMINAL_KEY_END:
        snprintf(buffer, buffer_size, "Key: End");
        break;

    default:
        if (key >= 32 && key <= 126) {
            snprintf(
                buffer,
                buffer_size,
                "Character: '%c'",
                key
            );
        } else {
            snprintf(
                buffer,
                buffer_size,
                "Key code: %d",
                key
            );
        }

        break;
    }
}

int tui_run(void)
{
    int key;
    int running = 1;

    char last_key[128] = "Press a key";

    if (terminal_init() != 0) {
        return 1;
    }

    draw_screen(last_key);

    while (running) {
        key = terminal_read_key();

        if (key < 0) {
            break;
        }

        switch (key) {
        case TERMINAL_KEY_RESIZE:
            snprintf(
                last_key,
                sizeof(last_key),
                "Terminal resized"
            );

            draw_screen(last_key);
            continue;

        case TERMINAL_KEY_TERMINATE:
            running = 0;
            continue;

        case TERMINAL_KEY_CTRL_C:
            running = 0;
            continue;

        default:
            break;
        }

        if (key == 'q' || key == 'Q') {
            running = 0;
            continue;
        }

        describe_key(
            key,
            last_key,
            sizeof(last_key)
        );

        draw_screen(last_key);
    }

    terminal_clear();
    terminal_move_cursor(1, 1);
    terminal_shutdown();

    return 0;
}
