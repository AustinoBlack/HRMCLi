#ifndef HRMCLI_UI_H
#define HRMCLI_UI_H

typedef struct {
    int row;
    int col;
    int height;
    int width;
} UiRect;

void ui_draw_text(
    int row,
    int col,
    const char *text
);

void ui_draw_centered_text(
    int row,
    int col,
    int width,
    const char *text
);

void ui_draw_horizontal_line(
    int row,
    int col,
    int width
);

void ui_draw_vertical_line(
    int row,
    int col,
    int height
);

void ui_draw_box(UiRect rect);

void ui_clear_rect(UiRect rect);

void ui_set_reverse(int enabled);

UiRect ui_get_screen_rect(void);

UiRect ui_rect_center(
    UiRect parent,
    int width,
    int height
);

UiRect ui_rect_inset(
    UiRect rect,
    int amount
);

void ui_split_vertical(
    UiRect parent,
    int percent,
    UiRect *left,
    UiRect *right
);

void ui_split_horizontal(
    UiRect parent,
    int percent,
    UiRect *top,
    UiRect *bottom
);

#endif
