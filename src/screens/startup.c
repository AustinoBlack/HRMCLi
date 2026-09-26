#include <stddef.h>
#include <stdio.h>

#include "hrmcli/screens/startup.h"
#include "hrmcli/ui.h"


static const char *startup_status_text(
    StartupStatus status
)
{
    switch (status) {
    case STARTUP_STATUS_OK:
        return "[ OK ]";

    case STARTUP_STATUS_WARN:
        return "[WARN]";

    case STARTUP_STATUS_FAIL:
        return "[FAIL]";

    case STARTUP_STATUS_PENDING:
    default:
        return "[....]";
    }
}


void screen_startup_draw(
    UiPane *pane,
    StartupState *state
)
{
    UiRect content;

    int row;

    if (
        pane == NULL ||
        state == NULL
    ) {
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

    /*
     * Header
     */
    ui_draw_centered_text(
        content.row,
        content.col,
        content.width,
        "HRMCLi"
    );

    ui_draw_centered_text(
        content.row + 2,
        content.col,
        content.width,
        "Homelab Rack Manager starting..."
    );

    row = content.row + 5;

    /*
     * Checklist
     */
    for (
        int i = 0;
        i < state->check_count;
        i++
    ) {
        StartupCheck *check;
        char line[256];

        check = &state->checks[i];

        if (
            row >=
            content.row + content.height - 3
        ) {
            break;
        }

        snprintf(
            line,
            sizeof(line),
            "%-6s  %s",
            startup_status_text(check->status),
            check->name
        );

        ui_draw_text(
            row,
            content.col + 2,
            line
        );

        row++;

        /*
         * Optional secondary message.
         */
        if (
            check->message[0] != '\0' &&
            row <
            content.row + content.height - 3
        ) {
            ui_draw_text(
                row,
                content.col + 10,
                check->message
            );

            row++;
        }
    }

    /*
     * Summary
     */
    row += 1;

    if (
        row <
        content.row + content.height
    ) {
        if (state->has_failures) {
            ui_draw_centered_text(
                row,
                content.col,
                content.width,
                "Startup checks failed."
            );
        } else if (state->has_warnings) {
            ui_draw_centered_text(
                row,
                content.col,
                content.width,
                "Startup completed with warnings."
            );
        } else {
            ui_draw_centered_text(
                row,
                content.col,
                content.width,
                "Startup checks complete."
            );
        }
    }
}
