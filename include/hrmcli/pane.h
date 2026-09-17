#ifndef HRMCLI_PANE_H
#define HRMCLI_PANE_H

#include "hrmcli/ui.h"

typedef struct UiPane UiPane;

typedef void (*UiPaneDrawFn)(
    UiPane *pane
);

typedef void (*UiPaneKeyFn)(
    UiPane *pane,
    int key
);

struct UiPane {
    UiRect rect;

    const char *title;

    int focused;
    int focusable;
    int visible;

    void *userdata;

    UiPaneDrawFn draw;
    UiPaneKeyFn handle_key;
};

void ui_pane_init(
    UiPane *pane,
    UiRect rect,
    const char *title
);

void ui_pane_set_rect(
    UiPane *pane,
    UiRect rect
);

void ui_pane_set_focus(
    UiPane *pane,
    int focused
);

void ui_pane_draw_frame(
    UiPane *pane
);

void ui_pane_draw(
    UiPane *pane
);

void ui_pane_handle_key(
    UiPane *pane,
    int key
);

#endif
