#include <stddef.h>
#include "hrmcli/screens/cli.h"
#include "hrmcli/ui.h"

void screen_cli_draw(
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
        "HRMCLi Command Line"
    );

    ui_draw_text(
        content.row + 4,
        content.col + 1,
        "HRMCLi> _"
    );
}

void screen_cli_handle_key(
    UiPane *pane,
    int key
)
{
    (void)pane;
    (void)key;

    /*
     * Actual CLI input handling will be
     * implemented later.
     */
}
