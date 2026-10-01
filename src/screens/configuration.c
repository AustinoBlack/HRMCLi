#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "hrmcli/paths.h"
#include "hrmcli/screens/configuration.h"
#include "hrmcli/serial.h"
#include "hrmcli/terminal.h"
#include "hrmcli/ui.h"


static const int baud_rates[] = {
    9600,
    19200,
    38400,
    57600,
    115200
};

#define BAUD_RATE_COUNT \
    ((int)(sizeof(baud_rates) / sizeof(baud_rates[0])))


static const char *parity_string(
    SerialParity parity
)
{
    switch (parity) {
    case SERIAL_PARITY_NONE:
        return "None";

    case SERIAL_PARITY_EVEN:
        return "Even";

    case SERIAL_PARITY_ODD:
        return "Odd";

    default:
        return "Unknown";
    }
}


static const char *flow_string(
    SerialFlowControl flow
)
{
    switch (flow) {
    case SERIAL_FLOW_NONE:
        return "None";

    case SERIAL_FLOW_SOFTWARE:
        return "Software";

    case SERIAL_FLOW_HARDWARE:
        return "Hardware";

    default:
        return "Unknown";
    }
}


static int find_baud_index(
    int baud
)
{
    for (int i = 0; i < BAUD_RATE_COUNT; i++) {
        if (baud_rates[i] == baud) {
            return i;
        }
    }

    return BAUD_RATE_COUNT - 1;
}


static int find_configured_device(
    ConfigurationScreenState *state
)
{
    if (state == NULL) {
        return -1;
    }

    if (state->config.device[0] == '\0') {
        return -1;
    }

    for (
        int i = 0;
        i < state->device_count;
        i++
    ) {
        if (
            strcmp(
                state->devices[i].path,
                state->config.device
            ) == 0
        ) {
            return i;
        }
    }

    return -1;
}

static void refresh_devices(
    ConfigurationScreenState *state
)
{
    char selected_path[SERIAL_DEVICE_PATH_MAX];
    int selected_index = -1;

    if (state == NULL) {
        return;
    }

    /*
     * Remember the currently selected device by
     * path before rebuilding the device list.
     */
    selected_path[0] = '\0';

    if (
        state->selected_device >= 0 &&
        state->selected_device < state->device_count
    ) {
        snprintf(
            selected_path,
            sizeof(selected_path),
            "%s",
            state->devices[
                state->selected_device
            ].path
        );
    }

    /*
     * Rebuild the device list.
     */
    if (
        serial_enumerate(
            state->devices,
            SERIAL_MAX_DEVICES,
            &state->device_count
        ) != 0
    ) {
        state->device_count = 0;
        state->selected_device = -1;

        snprintf(
            state->message,
            sizeof(state->message),
            "Failed to enumerate serial devices."
        );

        return;
    }

    /*
     * Prefer the configured device whenever it
     * is present.
     *
     * This allows a configured USB serial device
     * to become selected again after reconnecting.
     */
    selected_index =
        find_configured_device(state);

    /*
     * If the configured device is not present,
     * preserve the previous selection if possible.
     */
    if (
        selected_index < 0 &&
        selected_path[0] != '\0'
    ) {
        for (
            int i = 0;
            i < state->device_count;
            i++
        ) {
            if (
                strcmp(
                    state->devices[i].path,
                    selected_path
                ) == 0
            ) {
                selected_index = i;
                break;
            }
        }
    }

    /*
     * Otherwise select the first usable device.
     */
    if (selected_index < 0) {
        for (
            int i = 0;
            i < state->device_count;
            i++
        ) {
            if (state->devices[i].present) {
                selected_index = i;
                break;
            }
        }
    }

    state->selected_device =
        selected_index;

    snprintf(
        state->message,
        sizeof(state->message),
        "Serial devices refreshed."
    );
}

void screen_configuration_init(
    ConfigurationScreenState *state
)
{
    SerialConfigStatus status;
    int configured_index;

    if (state == NULL) {
        return;
    }

    memset(
        state,
        0,
        sizeof(*state)
    );

    serial_config_defaults(
        &state->config
    );

    status = serial_config_load(
        hrmcli_serial_config_path(),
        &state->config
    );

    /*
     * Missing config is expected on first run.
     * Defaults are already loaded.
     */
    if (
        status != SERIAL_CONFIG_OK &&
        status != SERIAL_CONFIG_FILE_NOT_FOUND
    ) {
        snprintf(
            state->message,
            sizeof(state->message),
            "%s",
            serial_config_status_string(status)
        );
    }

    if (
        serial_enumerate(
            state->devices,
            SERIAL_MAX_DEVICES,
            &state->device_count
        ) != 0
    ) {
        state->device_count = 0;

        snprintf(
            state->message,
            sizeof(state->message),
            "Failed to enumerate serial devices."
        );
    }

    configured_index =
        find_configured_device(state);

    if (configured_index >= 0) {
        state->selected_device =
            configured_index;
    } else {
        /*
         * Prefer the first available device.
         */
        state->selected_device = -1;

        for (
            int i = 0;
            i < state->device_count;
            i++
        ) {
            if (state->devices[i].present) {
                state->selected_device = i;
                break;
            }
        }
    }

    state->field =
        SERIAL_FIELD_DEVICE;
}


static void cycle_present_device(
    ConfigurationScreenState *state,
    int direction
)
{
    int index;

    if (
        state == NULL ||
        state->device_count <= 0
    ) {
        return;
    }

    index =
        state->selected_device;

    for (
        int attempt = 0;
        attempt < state->device_count;
        attempt++
    ) {
        index += direction;

        if (index < 0) {
            index =
                state->device_count - 1;
        }

        if (
            index >=
            state->device_count
        ) {
            index = 0;
        }

        if (
            state->devices[index].present
        ) {
            state->selected_device =
                index;

            return;
        }
    }
}


static void cycle_baud(
    ConfigurationScreenState *state,
    int direction
)
{
    int index;

    if (state == NULL) {
        return;
    }

    index =
        find_baud_index(
            state->config.baud
        );

    index += direction;

    if (index < 0) {
        index =
            BAUD_RATE_COUNT - 1;
    }

    if (index >= BAUD_RATE_COUNT) {
        index = 0;
    }

    state->config.baud =
        baud_rates[index];
}


static void save_configuration(
    ConfigurationScreenState *state
)
{
    SerialConfigStatus status;

    if (state == NULL) {
        return;
    }

    if (
        state->selected_device < 0 ||
        state->selected_device >=
            state->device_count
    ) {
        snprintf(
            state->message,
            sizeof(state->message),
            "No serial device selected."
        );

        return;
    }

    snprintf(
        state->config.device,
        sizeof(state->config.device),
        "%s",
        state->devices[
            state->selected_device
        ].path
    );

    status = serial_config_save(
        hrmcli_serial_config_path(),
        &state->config
    );

    if (status != SERIAL_CONFIG_OK) {
        snprintf(
            state->message,
            sizeof(state->message),
            "Save failed: %s",
            serial_config_status_string(
                status
            )
        );

        return;
    }

    snprintf(
        state->message,
        sizeof(state->message),
        "Serial configuration saved."
    );
}


void screen_configuration_draw(
    UiPane *pane,
    ConfigurationScreenState *state
)
{
    UiRect content;

    char line[384];

    int row;

    if (
        pane == NULL ||
        state == NULL
    ) {
        return;
    }

    content =
        ui_rect_inset(
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
        "Serial Console Configuration"
    );

    row =
        content.row + 3;

    /*
     * Device
     */
    if (
        state->field ==
        SERIAL_FIELD_DEVICE
    ) {
        ui_set_reverse(1);
    }

    if (
        state->selected_device >= 0 &&
        state->selected_device <
            state->device_count
    ) {
        snprintf(
            line,
            sizeof(line),
            "Device:       %s",
            state->devices[
                state->selected_device
            ].name
        );
    } else {
        snprintf(
            line,
            sizeof(line),
            "Device:       None"
        );
    }

    ui_draw_text(
        row++,
        content.col + 2,
        line
    );

    if (
        state->field ==
        SERIAL_FIELD_DEVICE
    ) {
        ui_set_reverse(0);
    }

    /*
    * Selected device details.
    */
    if (
        state->selected_device >= 0 &&
        state->selected_device <
        state->device_count
    ) {
        SerialDevice *device =
            &state->devices[
                state->selected_device
            ];

        snprintf(
            line,
            sizeof(line),
            "Status:       %s",
            device->present
                ? "Present"
                : "Missing"
        );

        ui_draw_text(
            row++,
            content.col + 4,
            line
        );

        snprintf(
            line,
            sizeof(line),
            "Access:       %s",
            device->accessible
                ? "Accessible"
                : "Inaccessible"
        );

        ui_draw_text(
            row++,
            content.col + 4,
            line
        );

        snprintf(
            line,
            sizeof(line),
            "Type:         %s%s",
            serial_device_type_string(
                device->type
            ),
            device->stable_path
                ? " [stable]"
                : ""
        );

        ui_draw_text(
            row++,
            content.col + 4,
            line
        );

        snprintf(
            line,
            sizeof(line),
            "Path:         %s",
            device->path
        );

        ui_draw_text(
            row++,
            content.col + 4,
            line
        );

        row++;
    }

    /*
     * Baud
     */
    if (
        state->field ==
        SERIAL_FIELD_BAUD
    ) {
        ui_set_reverse(1);
    }

    snprintf(
        line,
        sizeof(line),
        "Baud:         %d",
        state->config.baud
    );

    ui_draw_text(
        row++,
        content.col + 2,
        line
    );

    if (
        state->field ==
        SERIAL_FIELD_BAUD
    ) {
        ui_set_reverse(0);
    }

    /*
     * Data bits
     */
    if (
        state->field ==
        SERIAL_FIELD_DATA_BITS
    ) {
        ui_set_reverse(1);
    }

    snprintf(
        line,
        sizeof(line),
        "Data bits:    %d",
        state->config.data_bits
    );

    ui_draw_text(
        row++,
        content.col + 2,
        line
    );

    if (
        state->field ==
        SERIAL_FIELD_DATA_BITS
    ) {
        ui_set_reverse(0);
    }

    /*
     * Parity
     */
    if (
        state->field ==
        SERIAL_FIELD_PARITY
    ) {
        ui_set_reverse(1);
    }

    snprintf(
        line,
        sizeof(line),
        "Parity:       %s",
        parity_string(
            state->config.parity
        )
    );

    ui_draw_text(
        row++,
        content.col + 2,
        line
    );

    if (
        state->field ==
        SERIAL_FIELD_PARITY
    ) {
        ui_set_reverse(0);
    }

    /*
     * Stop bits
     */
    if (
        state->field ==
        SERIAL_FIELD_STOP_BITS
    ) {
        ui_set_reverse(1);
    }

    snprintf(
        line,
        sizeof(line),
        "Stop bits:    %d",
        state->config.stop_bits
    );

    ui_draw_text(
        row++,
        content.col + 2,
        line
    );

    if (
        state->field ==
        SERIAL_FIELD_STOP_BITS
    ) {
        ui_set_reverse(0);
    }

    /*
     * Flow control
     */
    if (
        state->field ==
        SERIAL_FIELD_FLOW_CONTROL
    ) {
        ui_set_reverse(1);
    }

    snprintf(
        line,
        sizeof(line),
        "Flow control: %s",
        flow_string(
            state->config.flow_control
        )
    );

    ui_draw_text(
        row++,
        content.col + 2,
        line
    );

    if (
        state->field ==
        SERIAL_FIELD_FLOW_CONTROL
    ) {
        ui_set_reverse(0);
    }

    row += 2;

    /*
     * Save button
     */
    if (
        state->field ==
        SERIAL_FIELD_SAVE
    ) {
        ui_set_reverse(1);
    }

    ui_draw_text(
        row,
        content.col + 2,
        "[ Save ]"
    );

    if (
        state->field ==
        SERIAL_FIELD_SAVE
    ) {
        ui_set_reverse(0);
    }

    if (
        state->message[0] != '\0'
    ) {
        ui_draw_text(
            row + 2,
            content.col + 2,
            state->message
        );
    }

    ui_draw_text(
        content.row +
        content.height - 1,
        content.col,
        "Tab Next   Left/Right Change   R Refresh   Enter Save   Esc Back"
    );
}


void screen_configuration_handle_key(
    ConfigurationScreenState *state,
    int key
)
{
    if (state == NULL) {
        return;
    }

    if (
        key == 'r' ||
        key == 'R'
    ) {
        refresh_devices(
            state
        );

        return;
    }

    if (key == TERMINAL_KEY_TAB) {
        state->field++;

        if (
            state->field >
            SERIAL_FIELD_SAVE
        ) {
            state->field =
                SERIAL_FIELD_DEVICE;
        }

        return;
    }

    if (
        key != TERMINAL_KEY_LEFT &&
        key != TERMINAL_KEY_RIGHT &&
        key != TERMINAL_KEY_ENTER
    ) {
        return;
    }

    switch (state->field) {
    case SERIAL_FIELD_DEVICE:
        if (key == TERMINAL_KEY_LEFT) {
            cycle_present_device(
                state,
                -1
            );
        } else if (
            key == TERMINAL_KEY_RIGHT
        ) {
            cycle_present_device(
                state,
                1
            );
        }
        break;

    case SERIAL_FIELD_BAUD:
        if (key == TERMINAL_KEY_LEFT) {
            cycle_baud(
                state,
                -1
            );
        } else if (
            key == TERMINAL_KEY_RIGHT
        ) {
            cycle_baud(
                state,
                1
            );
        }
        break;

    case SERIAL_FIELD_DATA_BITS:
        if (
            key == TERMINAL_KEY_LEFT ||
            key == TERMINAL_KEY_RIGHT
        ) {
            state->config.data_bits++;

            if (
                state->config.data_bits > 8
            ) {
                state->config.data_bits = 5;
            }
        }
        break;

    case SERIAL_FIELD_PARITY:
        if (
            key == TERMINAL_KEY_LEFT ||
            key == TERMINAL_KEY_RIGHT
        ) {
            state->config.parity++;

            if (
                state->config.parity >
                SERIAL_PARITY_ODD
            ) {
                state->config.parity =
                    SERIAL_PARITY_NONE;
            }
        }
        break;

    case SERIAL_FIELD_STOP_BITS:
        if (
            key == TERMINAL_KEY_LEFT ||
            key == TERMINAL_KEY_RIGHT
        ) {
            state->config.stop_bits =
                state->config.stop_bits == 1
                    ? 2
                    : 1;
        }
        break;

    case SERIAL_FIELD_FLOW_CONTROL:
        if (
            key == TERMINAL_KEY_LEFT ||
            key == TERMINAL_KEY_RIGHT
        ) {
            state->config.flow_control++;

            if (
                state->config.flow_control >
                SERIAL_FLOW_HARDWARE
            ) {
                state->config.flow_control =
                    SERIAL_FLOW_NONE;
            }
        }
        break;

    case SERIAL_FIELD_SAVE:
        if (key == TERMINAL_KEY_ENTER) {
            save_configuration(
                state
            );
        }
        break;
    }
}
