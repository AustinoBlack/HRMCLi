#include <stddef.h>
#include "hrmcli/screens/dashboard.h"
#include "hrmcli/ui.h"

void screen_dashboard_draw(
    UiPane *pane
)
{
    UiRect content;

    if (pane == NULL) {
        return;
    }

    content = ui_rect_inset(
        pane->rect,
        2
    );

    if (
        content.width <= 0 ||
        content.height <= 0
    ) {
        return;
    }

    ui_draw_centered_text(
        content.row + 2,
        content.col,
        content.width,
        "Dashboard"
    );

    ui_draw_centered_text(
        content.row + 4,
        content.col,
        content.width,
        "Dashboard functionality is not implemented yet."
    );

    ui_draw_centered_text(
        content.row + 6,
        content.col,
        content.width,
        "Press Esc to return."
    );
}

void screen_dashboard_handle_key(
    UiPane *pane,
    int key
)
{
    (void)pane;
    (void)key;
}
