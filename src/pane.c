#include <string.h>

#include "hrmcli/pane.h"
#include "hrmcli/ui.h"

void ui_pane_init(
    UiPane *pane,
    UiRect rect,
    const char *title
)
{
    if (pane == NULL) {
        return;
    }

    pane->rect = rect;
    pane->title = title;

    pane->focused = 0;
    pane->focusable = 1;
    pane->visible = 1;

    pane->userdata = NULL;

    pane->draw = NULL;
    pane->handle_key = NULL;
}


void ui_pane_set_rect(
    UiPane *pane,
    UiRect rect
)
{
    if (pane == NULL) {
        return;
    }

    pane->rect = rect;
}


void ui_pane_set_focus(
    UiPane *pane,
    int focused
)
{
    if (pane == NULL) {
        return;
    }

    if (!pane->focusable) {
        pane->focused = 0;
        return;
    }

    pane->focused = focused ? 1 : 0;
}


void ui_pane_draw_frame(
    UiPane *pane
)
{
    int title_length;
    int title_col;

    if (
        pane == NULL ||
        !pane->visible
    ) {
        return;
    }

    ui_draw_box(pane->rect);

    if (
        pane->title == NULL ||
        pane->rect.width < 4
    ) {
        return;
    }

    title_length = (int)strlen(pane->title);

    title_col =
        pane->rect.col +
        ((pane->rect.width - title_length) / 2);

    if (title_col <= pane->rect.col) {
        title_col = pane->rect.col + 1;
    }

    /*
     * Highlight the title when this pane has focus.
     */
    if (pane->focused) {
        ui_set_reverse(1);
    }

    ui_draw_text(
        pane->rect.row,
        title_col,
        pane->title
    );

    if (pane->focused) {
        ui_set_reverse(0);
    }
}


void ui_pane_draw(
    UiPane *pane
)
{
    if (
        pane == NULL ||
        !pane->visible
    ) {
        return;
    }

    ui_pane_draw_frame(pane);

    if (pane->draw != NULL) {
        pane->draw(pane);
    }
}


void ui_pane_handle_key(
    UiPane *pane,
    int key
)
{
    if (
        pane == NULL ||
        !pane->visible ||
        !pane->focused
    ) {
        return;
    }

    if (pane->handle_key != NULL) {
        pane->handle_key(
            pane,
            key
        );
    }
}
