#include <stddef.h>
#include <stdio.h>

#include "hrmcli/screens/configuration.h"
#include "hrmcli/serial.h"
#include "hrmcli/ui.h"


void screen_configuration_draw(
    UiPane *pane
)
{
    UiRect content;

    SerialDevice devices[SERIAL_MAX_DEVICES];

    int device_count = 0;
    int available_count = 0;
    int unavailable_count = 0;

    int row;

    char line[384];

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
        content.row,
        content.col,
        content.width,
        "Configuration"
    );

    row =
        content.row + 2;

    ui_draw_text(
        row++,
        content.col,
        "Serial Console"
    );

    row++;

    if (
        serial_enumerate(
            devices,
            SERIAL_MAX_DEVICES,
            &device_count
        ) != 0
    ) {
        ui_draw_text(
            row,
            content.col + 2,
            "Failed to enumerate serial devices."
        );

        return;
    }

    for (
        int i = 0;
        i < device_count;
        i++
    ) {
        if (devices[i].available) {
            available_count++;
        } else {
            unavailable_count++;
        }
    }

    if (available_count == 0) {
        ui_draw_text(
            row++,
            content.col + 2,
            "No usable serial devices detected."
        );
    } else {
        snprintf(
            line,
            sizeof(line),
            "Detected usable devices: %d",
            available_count
        );

        ui_draw_text(
            row++,
            content.col + 2,
            line
        );

        row++;

        for (
            int i = 0;
            i < device_count;
            i++
        ) {
            if (!devices[i].available) {
                continue;
            }

            snprintf(
                line,
                sizeof(line),
                "%s%s",
                devices[i].name,
                devices[i].stable_path
                    ? " [stable]"
                    : ""
            );

            ui_draw_text(
                row++,
                content.col + 2,
                line
            );

            snprintf(
                line,
                sizeof(line),
                "  Path: %s",
                devices[i].path
            );

            ui_draw_text(
                row++,
                content.col + 2,
                line
            );

            snprintf(
                line,
                sizeof(line),
                "  Type: %s   Status: Available",
                serial_device_type_string(
                    devices[i].type
                )
            );

            ui_draw_text(
                row++,
                content.col + 2,
                line
            );

            row++;

            if (
                row >=
                content.row +
                content.height - 3
            ) {
                break;
            }
        }
    }

    if (unavailable_count > 0) {
        snprintf(
            line,
            sizeof(line),
            "%d additional serial candidate%s unavailable.",
            unavailable_count,
            unavailable_count == 1
                ? ""
                : "s"
        );

        ui_draw_text(
            content.row +
            content.height - 2,
            content.col,
            line
        );
    }

    ui_draw_text(
        content.row +
        content.height - 1,
        content.col,
        "Esc Back"
    );
}


void screen_configuration_handle_key(
    UiPane *pane,
    int key
)
{
    (void)pane;
    (void)key;
}
