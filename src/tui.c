#include <stdio.h>
#include <string.h>

#include "hrmcli/terminal.h"
#include "hrmcli/tui.h"
#include "hrmcli/ui.h"
/*
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
*/

static void draw_screen(const char *last_event)
{
    UiRect screen;
    UiRect content;
    UiRect left;
    UiRect right;
    UiRect test_box;

    screen = ui_get_screen_rect();

    if (
        screen.width <= 0 ||
        screen.height <= 0
    ) {
        return;
    }

    terminal_clear();

    /*
     * Outer application boundary.
     */
    ui_draw_box(screen);

    /*
     * Leave one cell between the application
     * boundary and our content.
     */
    content = ui_rect_inset(
        screen,
        2
    );

    if (
        content.width <= 0 ||
        content.height <= 0
    ) {
        return;
    }

    /*
     * Split the available content 40 / 60.
     */
    ui_split_vertical(
        content,
        40,
        &left,
        &right
    );

    /*
     * Draw the two resulting regions.
     */
    ui_draw_box(left);
    ui_draw_box(right);

    ui_draw_centered_text(
        left.row + 1,
        left.col + 1,
        left.width - 2,
        "PRIMARY"
    );

    ui_draw_centered_text(
        right.row + 1,
        right.col + 1,
        right.width - 2,
        "SECONDARY"
    );

    /*
     * Test centering inside the left pane.
     */
    test_box = ui_rect_center(
        ui_rect_inset(left, 2),
        24,
        7
    );

    ui_draw_box(test_box);

    ui_draw_centered_text(
        test_box.row + 2,
        test_box.col + 1,
        test_box.width - 2,
        "HRMCLi"
    );

    ui_draw_centered_text(
        test_box.row + 4,
        test_box.col + 1,
        test_box.width - 2,
        last_event
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
