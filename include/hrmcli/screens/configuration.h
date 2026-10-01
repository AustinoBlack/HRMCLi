#ifndef HRMCLI_SCREEN_CONFIGURATION_H
#define HRMCLI_SCREEN_CONFIGURATION_H

#include "hrmcli/pane.h"
#include "hrmcli/serial.h"

#define CONFIGURATION_MESSAGE_MAX 128

typedef enum {
    SERIAL_FIELD_DEVICE,
    SERIAL_FIELD_BAUD,
    SERIAL_FIELD_DATA_BITS,
    SERIAL_FIELD_PARITY,
    SERIAL_FIELD_STOP_BITS,
    SERIAL_FIELD_FLOW_CONTROL,
    SERIAL_FIELD_SAVE
} SerialConfigField;

typedef struct {
    SerialDevice devices[SERIAL_MAX_DEVICES];

    int device_count;
    int selected_device;

    SerialConfig config;

    SerialConfigField field;

    char message[CONFIGURATION_MESSAGE_MAX];
} ConfigurationScreenState;

void screen_configuration_init(
    ConfigurationScreenState *state
);

void screen_configuration_draw(
    UiPane *pane,
    ConfigurationScreenState *state
);

void screen_configuration_handle_key(
    ConfigurationScreenState *state,
    int key
);

#endif
