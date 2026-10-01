#ifndef HRMCLI_SERIAL_H
#define HRMCLI_SERIAL_H

#define SERIAL_DEVICE_PATH_MAX 256
#define SERIAL_DEVICE_NAME_MAX 128
#define SERIAL_MAX_DEVICES 32

typedef enum {
    SERIAL_DEVICE_UART,
    SERIAL_DEVICE_USB,
    SERIAL_DEVICE_ACM,
    SERIAL_DEVICE_UNKNOWN
} SerialDeviceType;

typedef struct {
    char path[SERIAL_DEVICE_PATH_MAX];
    char name[SERIAL_DEVICE_NAME_MAX];

    SerialDeviceType type;

    int stable_path;
    int present;
    int accessible; /*avaiable -> accessible Clearer terminology*/
} SerialDevice;

typedef enum {
    SERIAL_PARITY_NONE,
    SERIAL_PARITY_EVEN,
    SERIAL_PARITY_ODD
} SerialParity;

typedef enum {
    SERIAL_FLOW_NONE,
    SERIAL_FLOW_SOFTWARE,
    SERIAL_FLOW_HARDWARE
} SerialFlowControl;

typedef struct {
    char device[SERIAL_DEVICE_PATH_MAX];

    int baud;
    int data_bits;
    int stop_bits;

    SerialParity parity;
    SerialFlowControl flow_control;
} SerialConfig;

typedef enum {
    SERIAL_CONFIG_OK = 0,

    SERIAL_CONFIG_FILE_NOT_FOUND,
    SERIAL_CONFIG_FILE_READ_ERROR,
    SERIAL_CONFIG_FILE_WRITE_ERROR,

    SERIAL_CONFIG_PARSE_ERROR,
    SERIAL_CONFIG_INVALID_ROOT,
    SERIAL_CONFIG_INVALID_VALUE,
    SERIAL_CONFIG_SERIALIZE_ERROR,

    SERIAL_CONFIG_INVALID_ARGUMENT
} SerialConfigStatus;

int serial_enumerate(
    SerialDevice *devices,
    int max_devices,
    int *device_count
);

const char *serial_device_type_string(
    SerialDeviceType type
);

void serial_config_defaults(
    SerialConfig *config
);

SerialConfigStatus serial_config_load(
    const char *path,
    SerialConfig *config
);

SerialConfigStatus serial_config_save(
    const char *path,
    const SerialConfig *config
);

const char *serial_config_status_string(
    SerialConfigStatus status
);

#endif
