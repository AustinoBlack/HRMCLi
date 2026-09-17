#include <stdio.h>
#include <string.h>

#include "hrmcli/terminal.h"
#include "hrmcli/ui.h"

void ui_draw_text(
    int row,
    int col,
    const char *text
)
{
    if (text == NULL) {
        return;
    }

    terminal_move_cursor(row, col);
    terminal_write(text);
}

void ui_draw_centered_text(
    int row,
    int col,
    int width,
    const char *text
)
{
    int length;
    int text_col;

    if (text == NULL || width <= 0) {
        return;
    }

    length = (int)strlen(text);

    text_col = col + ((width - length) / 2);

    if (text_col < col) {
        text_col = col;
    }

    ui_draw_text(
        row,
        text_col,
        text
    );
}

void ui_draw_horizontal_line(
    int row,
    int col,
    int width
)
{
    if (width <= 0) {
        return;
    }

    terminal_move_cursor(row, col);

    for (int i = 0; i < width; i++) {
        terminal_write("-");
    }
}

void ui_draw_vertical_line(
    int row,
    int col,
    int height
)
{
    if (height <= 0) {
        return;
    }

    for (int i = 0; i < height; i++) {
        terminal_move_cursor(
            row + i,
            col
        );

        terminal_write("|");
    }
}

void ui_draw_box(UiRect rect)
{
    if (
        rect.width < 2 ||
        rect.height < 2
    ) {
        return;
    }

    /*
     * Corners
     */
    ui_draw_text(
        rect.row,
        rect.col,
        "+"
    );

    ui_draw_text(
        rect.row,
        rect.col + rect.width - 1,
        "+"
    );

    ui_draw_text(
        rect.row + rect.height - 1,
        rect.col,
        "+"
    );

    ui_draw_text(
        rect.row + rect.height - 1,
        rect.col + rect.width - 1,
        "+"
    );

    /*
     * Horizontal edges
     */
    ui_draw_horizontal_line(
        rect.row,
        rect.col + 1,
        rect.width - 2
    );

    ui_draw_horizontal_line(
        rect.row + rect.height - 1,
        rect.col + 1,
        rect.width - 2
    );

    /*
     * Vertical edges
     */
    ui_draw_vertical_line(
        rect.row + 1,
        rect.col,
        rect.height - 2
    );

    ui_draw_vertical_line(
        rect.row + 1,
        rect.col + rect.width - 1,
        rect.height - 2
    );
}

void ui_clear_rect(UiRect rect)
{
    for (int row = 0; row < rect.height; row++) {
        terminal_move_cursor(
            rect.row + row,
            rect.col
        );

        for (int col = 0; col < rect.width; col++) {
            terminal_write(" ");
        }
    }
}

void ui_set_reverse(int enabled)
{
    terminal_set_reverse(enabled);
}

UiRect ui_get_screen_rect(void)
{
    TerminalSize size;
    UiRect rect = {0};

    if (terminal_get_size(&size) != 0) {
        return rect;
    }

    rect.row = 1;
    rect.col = 1;
    rect.height = size.rows;
    rect.width = size.cols;

    return rect;
}


UiRect ui_rect_center(
    UiRect parent,
    int width,
    int height
)
{
    UiRect rect = {0};

    if (
        parent.width <= 0 ||
        parent.height <= 0 ||
        width <= 0 ||
        height <= 0
    ) {
        return rect;
    }

    /*
     * Do not allow the child rectangle to grow
     * outside its parent.
     */
    if (width > parent.width) {
        width = parent.width;
    }

    if (height > parent.height) {
        height = parent.height;
    }

    rect.width = width;
    rect.height = height;

    rect.row =
        parent.row +
        ((parent.height - height) / 2);

    rect.col =
        parent.col +
        ((parent.width - width) / 2);

    return rect;
}

UiRect ui_rect_inset(
    UiRect rect,
    int amount
)
{
    UiRect result = {0};

    if (
        rect.width <= 0 ||
        rect.height <= 0
    ) {
        return result;
    }

    if (amount < 0) {
        amount = 0;
    }

    /*
     * An inset removes 'amount' cells from
     * every side, so it consumes twice that
     * amount from both dimensions.
     */
    if (
        rect.width <= amount * 2 ||
        rect.height <= amount * 2
    ) {
        return result;
    }

    result.row = rect.row + amount;
    result.col = rect.col + amount;

    result.width =
        rect.width - (amount * 2);

    result.height =
        rect.height - (amount * 2);

    return result;
}


void ui_split_vertical(
    UiRect parent,
    int percent,
    UiRect *left,
    UiRect *right
)
{
    int left_width;

    if (left == NULL || right == NULL) {
        return;
    }

    *left = (UiRect){0};
    *right = (UiRect){0};

    if (
        parent.width < 2 ||
        parent.height <= 0
    ) {
        return;
    }

    if (percent < 1) {
        percent = 1;
    }

    if (percent > 99) {
        percent = 99;
    }

    left_width =
        (parent.width * percent) / 100;

    /*
     * Both sides must receive at least one column.
     */
    if (left_width < 1) {
        left_width = 1;
    }

    if (left_width >= parent.width) {
        left_width = parent.width - 1;
    }

    left->row = parent.row;
    left->col = parent.col;
    left->height = parent.height;
    left->width = left_width;

    right->row = parent.row;
    right->col = parent.col + left_width;
    right->height = parent.height;
    right->width = parent.width - left_width;
}


void ui_split_horizontal(
    UiRect parent,
    int percent,
    UiRect *top,
    UiRect *bottom
)
{
    int top_height;

    if (top == NULL || bottom == NULL) {
        return;
    }

    *top = (UiRect){0};
    *bottom = (UiRect){0};

    if (
        parent.height < 2 ||
        parent.width <= 0
    ) {
        return;
    }

    if (percent < 1) {
        percent = 1;
    }

    if (percent > 99) {
        percent = 99;
    }

    top_height =
        (parent.height * percent) / 100;

    if (top_height < 1) {
        top_height = 1;
    }

    if (top_height >= parent.height) {
        top_height = parent.height - 1;
    }

    top->row = parent.row;
    top->col = parent.col;
    top->height = top_height;
    top->width = parent.width;

    bottom->row = parent.row + top_height;
    bottom->col = parent.col;
    bottom->height =
        parent.height - top_height;
    bottom->width = parent.width;
}
