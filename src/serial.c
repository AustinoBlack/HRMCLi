#define _XOPEN_SOURCE 700

#include <glob.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <strings.h>
#include <cjson/cJSON.h>

#include "hrmcli/serial.h"


static const char *serial_parity_config_string(
    SerialParity parity
)
{
    switch (parity) {
    case SERIAL_PARITY_NONE:
        return "none";

    case SERIAL_PARITY_EVEN:
        return "even";

    case SERIAL_PARITY_ODD:
        return "odd";

    default:
        return NULL;
    }
}


static const char *serial_flow_config_string(
    SerialFlowControl flow
)
{
    switch (flow) {
    case SERIAL_FLOW_NONE:
        return "none";

    case SERIAL_FLOW_SOFTWARE:
        return "software";

    case SERIAL_FLOW_HARDWARE:
        return "hardware";

    default:
        return NULL;
    }
}


static int parse_serial_parity(
    const char *text,
    SerialParity *parity
)
{
    if (
        text == NULL ||
        parity == NULL
    ) {
        return -1;
    }

    if (strcasecmp(text, "none") == 0) {
        *parity = SERIAL_PARITY_NONE;
        return 0;
    }

    if (strcasecmp(text, "even") == 0) {
        *parity = SERIAL_PARITY_EVEN;
        return 0;
    }

    if (strcasecmp(text, "odd") == 0) {
        *parity = SERIAL_PARITY_ODD;
        return 0;
    }

    return -1;
}


static int parse_serial_flow(
    const char *text,
    SerialFlowControl *flow
)
{
    if (
        text == NULL ||
        flow == NULL
    ) {
        return -1;
    }

    if (strcasecmp(text, "none") == 0) {
        *flow = SERIAL_FLOW_NONE;
        return 0;
    }

    if (strcasecmp(text, "software") == 0) {
        *flow = SERIAL_FLOW_SOFTWARE;
        return 0;
    }

    if (strcasecmp(text, "hardware") == 0) {
        *flow = SERIAL_FLOW_HARDWARE;
        return 0;
    }

    return -1;
}


static SerialConfigStatus serial_config_read_file(
    const char *path,
    char **buffer
)
{
    FILE *file;
    long size;
    size_t bytes_read;
    char *data;

    if (
        path == NULL ||
        buffer == NULL
    ) {
        return SERIAL_CONFIG_INVALID_ARGUMENT;
    }

    *buffer = NULL;

    file = fopen(
        path,
        "rb"
    );

    if (file == NULL) {
        if (errno == ENOENT) {
            return SERIAL_CONFIG_FILE_NOT_FOUND;
        }

        return SERIAL_CONFIG_FILE_READ_ERROR;
    }

    if (
        fseek(
            file,
            0,
            SEEK_END
        ) != 0
    ) {
        fclose(file);
        return SERIAL_CONFIG_FILE_READ_ERROR;
    }

    size = ftell(file);

    if (size < 0) {
        fclose(file);
        return SERIAL_CONFIG_FILE_READ_ERROR;
    }

    rewind(file);

    data = malloc(
        (size_t)size + 1
    );

    if (data == NULL) {
        fclose(file);
        return SERIAL_CONFIG_FILE_READ_ERROR;
    }

    bytes_read = fread(
        data,
        1,
        (size_t)size,
        file
    );

    if (
        bytes_read != (size_t)size &&
        ferror(file)
    ) {
        free(data);
        fclose(file);

        return SERIAL_CONFIG_FILE_READ_ERROR;
    }

    data[bytes_read] = '\0';

    fclose(file);

    *buffer = data;

    return SERIAL_CONFIG_OK;
}


void serial_config_defaults(
    SerialConfig *config
)
{
    if (config == NULL) {
        return;
    }

    memset(
        config,
        0,
        sizeof(*config)
    );

    config->baud = 115200;
    config->data_bits = 8;
    config->stop_bits = 1;
    config->parity = SERIAL_PARITY_NONE;
    config->flow_control = SERIAL_FLOW_NONE;
}


static int serial_device_is_accessible(
    const char *path
)
{
    int fd;

    if (path == NULL) {
        return 0;
    }

    fd = open(
        path,
        O_RDWR |
        O_NOCTTY |
        O_NONBLOCK
    );

    if (fd < 0) {
        return 0;
    }

    close(fd);

    return 1;
}


static int add_device(
    SerialDevice *devices,
    int max_devices,
    int *device_count,
    const char *path,
    const char *name,
    SerialDeviceType type,
    int stable_path
)
{
    SerialDevice *device;

    if (
        devices == NULL ||
        device_count == NULL ||
        path == NULL ||
        max_devices <= 0
    ) {
        return -1;
    }

    if (*device_count >= max_devices) {
        return 0;
    }

    device =
        &devices[*device_count];

    memset(
        device,
        0,
        sizeof(*device)
    );

    if (
        snprintf(
            device->path,
            sizeof(device->path),
            "%s",
            path
        ) >=
        (int)sizeof(device->path)
    ) {
        return 0;
    }

    if (name != NULL) {
        snprintf(
            device->name,
            sizeof(device->name),
            "%s",
            name
        );
    }

    device->type =
        type;

    device->stable_path =
        stable_path ? 1 : 0;

    /*
     * add_device() is only called for devices
     * discovered by enumeration, so they are
     * present in the current Linux device tree.
     */
    device->present = 1;

    /*
     * Accessible means HRMCLi can directly open
     * the device. This is deliberately separate
     * from presence because permissions or another
     * process may prevent access to a valid device.
     */
    device->accessible =
        serial_device_is_accessible(
            device->path
        );

    (*device_count)++;

    return 0;
}


static int device_target_already_present(
    const SerialDevice *devices,
    int device_count,
    const char *path
)
{
    char candidate_real[SERIAL_DEVICE_PATH_MAX];

    if (
        devices == NULL ||
        path == NULL ||
        device_count <= 0
    ) {
        return 0;
    }

    if (
        realpath(
            path,
            candidate_real
        ) == NULL
    ) {
        return 0;
    }

    for (int i = 0; i < device_count; i++) {
        char existing_real[SERIAL_DEVICE_PATH_MAX];

        if (
            realpath(
                devices[i].path,
                existing_real
            ) == NULL
        ) {
            continue;
        }

        if (
            strcmp(
                candidate_real,
                existing_real
            ) == 0
        ) {
            return 1;
        }
    }

    return 0;
}


static int enumerate_pattern(
    SerialDevice *devices,
    int max_devices,
    int *device_count,
    const char *pattern,
    SerialDeviceType type,
    int stable_path
)
{
    glob_t matches;
    int result;

    memset(
        &matches,
        0,
        sizeof(matches)
    );

    result =
        glob(
            pattern,
            0,
            NULL,
            &matches
        );

    if (result == GLOB_NOMATCH) {
        globfree(&matches);
        return 0;
    }

    if (result != 0) {
        globfree(&matches);
        return -1;
    }

    for (
        size_t i = 0;
        i < matches.gl_pathc;
        i++
    ) {
        const char *path;
        const char *name;
        const char *slash;

        path =
            matches.gl_pathv[i];

        slash =
            strrchr(
                path,
                '/'
            );

        if (slash != NULL) {
            name = slash + 1;
        } else {
            name = path;
        }

        if (
            !stable_path &&
            device_target_already_present(
                devices,
                *device_count,
                path
            )
        ) {
            continue;
        }

        if (
            add_device(
                devices,
                max_devices,
                device_count,
                path,
                name,
                type,
                stable_path
            ) != 0
        ) {
            globfree(&matches);
            return -1;
        }

        if (
            *device_count >=
            max_devices
        ) {
            break;
        }
    }

    globfree(&matches);

    return 0;
}


int serial_enumerate(
    SerialDevice *devices,
    int max_devices,
    int *device_count
)
{
    if (
        devices == NULL ||
        device_count == NULL ||
        max_devices <= 0
    ) {
        return -1;
    }

    *device_count = 0;

    /*
     * Prefer stable USB device identifiers first.
     */
    if (
        enumerate_pattern(
            devices,
            max_devices,
            device_count,
            "/dev/serial/by-id/*",
            SERIAL_DEVICE_USB,
            1
        ) != 0
    ) {
        return -1;
    }

    /*
     * Common USB serial devices.
     */
    if (
        enumerate_pattern(
            devices,
            max_devices,
            device_count,
            "/dev/ttyUSB*",
            SERIAL_DEVICE_USB,
            0
        ) != 0
    ) {
        return -1;
    }

    /*
     * USB CDC ACM devices.
     */
    if (
        enumerate_pattern(
            devices,
            max_devices,
            device_count,
            "/dev/ttyACM*",
            SERIAL_DEVICE_ACM,
            0
        ) != 0
    ) {
        return -1;
    }

    /*
     * Traditional onboard UARTs.
     */
    if (
        enumerate_pattern(
            devices,
            max_devices,
            device_count,
            "/dev/ttyS*",
            SERIAL_DEVICE_UART,
            0
        ) != 0
    ) {
        return -1;
    }

    return 0;
}


const char *serial_device_type_string(
    SerialDeviceType type
)
{
    switch (type) {
    case SERIAL_DEVICE_UART:
        return "UART";

    case SERIAL_DEVICE_USB:
        return "USB";

    case SERIAL_DEVICE_ACM:
        return "ACM";

    case SERIAL_DEVICE_UNKNOWN:
    default:
        return "Unknown";
    }
}


SerialConfigStatus serial_config_load(
    const char *path,
    SerialConfig *config
)
{
    char *json_data;

    cJSON *root;
    cJSON *serial;
    cJSON *device;
    cJSON *baud;
    cJSON *data_bits;
    cJSON *parity;
    cJSON *stop_bits;
    cJSON *flow_control;

    SerialConfig parsed;

    SerialConfigStatus status;

    if (
        path == NULL ||
        config == NULL
    ) {
        return SERIAL_CONFIG_INVALID_ARGUMENT;
    }

    /*
     * Start with known-safe defaults.
     *
     * This also means a missing serial.json still
     * leaves the caller with a usable default config.
     */
    serial_config_defaults(
        config
    );

    status = serial_config_read_file(
        path,
        &json_data
    );

    if (status != SERIAL_CONFIG_OK) {
        return status;
    }

    root = cJSON_Parse(
        json_data
    );

    free(json_data);

    if (root == NULL) {
        return SERIAL_CONFIG_PARSE_ERROR;
    }

    if (!cJSON_IsObject(root)) {
        cJSON_Delete(root);
        return SERIAL_CONFIG_INVALID_ROOT;
    }

    serial =
        cJSON_GetObjectItemCaseSensitive(
            root,
            "serial"
        );

    if (!cJSON_IsObject(serial)) {
        cJSON_Delete(root);
        return SERIAL_CONFIG_INVALID_ROOT;
    }

    device =
        cJSON_GetObjectItemCaseSensitive(
            serial,
            "device"
        );

    baud =
        cJSON_GetObjectItemCaseSensitive(
            serial,
            "baud"
        );

    data_bits =
        cJSON_GetObjectItemCaseSensitive(
            serial,
            "data_bits"
        );

    parity =
        cJSON_GetObjectItemCaseSensitive(
            serial,
            "parity"
        );

    stop_bits =
        cJSON_GetObjectItemCaseSensitive(
            serial,
            "stop_bits"
        );

    flow_control =
        cJSON_GetObjectItemCaseSensitive(
            serial,
            "flow_control"
        );

    /*
     * Every field must exist and have the
     * expected JSON type.
     */
    if (
        !cJSON_IsString(device) ||
        !cJSON_IsNumber(baud) ||
        !cJSON_IsNumber(data_bits) ||
        !cJSON_IsString(parity) ||
        !cJSON_IsNumber(stop_bits) ||
        !cJSON_IsString(flow_control)
    ) {
        cJSON_Delete(root);
        return SERIAL_CONFIG_INVALID_VALUE;
    }

    /*
     * Reject an overlong device path rather
     * than silently truncating it.
     *
     * An empty path is allowed and means
     * "not configured".
     */
    if (
        strlen(device->valuestring) >=
        SERIAL_DEVICE_PATH_MAX
    ) {
        cJSON_Delete(root);
        return SERIAL_CONFIG_INVALID_VALUE;
    }

    /*
     * Parse into a temporary structure so a bad
     * file never partially modifies the caller's
     * configuration.
     */
    serial_config_defaults(
        &parsed
    );

    snprintf(
        parsed.device,
        sizeof(parsed.device),
        "%s",
        device->valuestring
    );

    parsed.baud =
        baud->valueint;

    parsed.data_bits =
        data_bits->valueint;

    parsed.stop_bits =
        stop_bits->valueint;

    if (
        parse_serial_parity(
            parity->valuestring,
            &parsed.parity
        ) != 0
    ) {
        cJSON_Delete(root);
        return SERIAL_CONFIG_INVALID_VALUE;
    }

    if (
        parse_serial_flow(
            flow_control->valuestring,
            &parsed.flow_control
        ) != 0
    ) {
        cJSON_Delete(root);
        return SERIAL_CONFIG_INVALID_VALUE;
    }

    /*
     * Basic serial-setting validation.
     */
    if (parsed.baud <= 0) {
        cJSON_Delete(root);
        return SERIAL_CONFIG_INVALID_VALUE;
    }

    if (
        parsed.data_bits < 5 ||
        parsed.data_bits > 8
    ) {
        cJSON_Delete(root);
        return SERIAL_CONFIG_INVALID_VALUE;
    }

    if (
        parsed.stop_bits != 1 &&
        parsed.stop_bits != 2
    ) {
        cJSON_Delete(root);
        return SERIAL_CONFIG_INVALID_VALUE;
    }

    cJSON_Delete(root);

    *config = parsed;

    return SERIAL_CONFIG_OK;
}


SerialConfigStatus serial_config_save(
    const char *path,
    const SerialConfig *config
)
{
    cJSON *root;
    cJSON *serial;

    const char *parity;
    const char *flow;

    char *json_data;
    char *tmp_path;

    FILE *file;

    size_t path_length;
    size_t json_length;
    size_t bytes_written;

    if (
        path == NULL ||
        config == NULL
    ) {
        return SERIAL_CONFIG_INVALID_ARGUMENT;
    }

    parity =
        serial_parity_config_string(
            config->parity
        );

    flow =
        serial_flow_config_string(
            config->flow_control
        );

    /*
     * Validate before constructing JSON.
     */
    if (
        strlen(config->device) >=
        SERIAL_DEVICE_PATH_MAX ||
        config->baud <= 0 ||
        config->data_bits < 5 ||
        config->data_bits > 8 ||
        (
            config->stop_bits != 1 &&
            config->stop_bits != 2
        ) ||
        parity == NULL ||
        flow == NULL
    ) {
        return SERIAL_CONFIG_INVALID_VALUE;
    }

    root = cJSON_CreateObject();

    if (root == NULL) {
        return SERIAL_CONFIG_SERIALIZE_ERROR;
    }

    serial = cJSON_CreateObject();

    if (serial == NULL) {
        cJSON_Delete(root);
        return SERIAL_CONFIG_SERIALIZE_ERROR;
    }

    if (
        !cJSON_AddItemToObject(
            root,
            "serial",
            serial
        )
    ) {
        cJSON_Delete(serial);
        cJSON_Delete(root);

        return SERIAL_CONFIG_SERIALIZE_ERROR;
    }

    if (
        cJSON_AddStringToObject(
            serial,
            "device",
            config->device
        ) == NULL ||

        cJSON_AddNumberToObject(
            serial,
            "baud",
            config->baud
        ) == NULL ||

        cJSON_AddNumberToObject(
            serial,
            "data_bits",
            config->data_bits
        ) == NULL ||

        cJSON_AddStringToObject(
            serial,
            "parity",
            parity
        ) == NULL ||

        cJSON_AddNumberToObject(
            serial,
            "stop_bits",
            config->stop_bits
        ) == NULL ||

        cJSON_AddStringToObject(
            serial,
            "flow_control",
            flow
        ) == NULL
    ) {
        cJSON_Delete(root);
        return SERIAL_CONFIG_SERIALIZE_ERROR;
    }

    json_data =
        cJSON_Print(root);

    cJSON_Delete(root);

    if (json_data == NULL) {
        return SERIAL_CONFIG_SERIALIZE_ERROR;
    }

    /*
     * Write to a temporary file first, then
     * atomically replace the live config.
     */
    path_length =
        strlen(path);

    tmp_path = malloc(
        path_length + 5
    );

    if (tmp_path == NULL) {
        cJSON_free(json_data);
        return SERIAL_CONFIG_FILE_WRITE_ERROR;
    }

    snprintf(
        tmp_path,
        path_length + 5,
        "%s.tmp",
        path
    );

    file = fopen(
        tmp_path,
        "wb"
    );

    if (file == NULL) {
        free(tmp_path);
        cJSON_free(json_data);

        return SERIAL_CONFIG_FILE_WRITE_ERROR;
    }

    json_length =
        strlen(json_data);

    bytes_written = fwrite(
        json_data,
        1,
        json_length,
        file
    );

    if (
        bytes_written != json_length ||
        fflush(file) != 0
    ) {
        fclose(file);
        remove(tmp_path);

        free(tmp_path);
        cJSON_free(json_data);

        return SERIAL_CONFIG_FILE_WRITE_ERROR;
    }

    if (fclose(file) != 0) {
        remove(tmp_path);

        free(tmp_path);
        cJSON_free(json_data);

        return SERIAL_CONFIG_FILE_WRITE_ERROR;
    }

    if (
        rename(
            tmp_path,
            path
        ) != 0
    ) {
        remove(tmp_path);

        free(tmp_path);
        cJSON_free(json_data);

        return SERIAL_CONFIG_FILE_WRITE_ERROR;
    }

    free(tmp_path);
    cJSON_free(json_data);

    return SERIAL_CONFIG_OK;
}


const char *serial_config_status_string(
    SerialConfigStatus status
)
{
    switch (status) {
    case SERIAL_CONFIG_OK:
        return "OK";

    case SERIAL_CONFIG_FILE_NOT_FOUND:
        return "File not found";

    case SERIAL_CONFIG_FILE_READ_ERROR:
        return "File read error";

    case SERIAL_CONFIG_FILE_WRITE_ERROR:
        return "File write error";

    case SERIAL_CONFIG_PARSE_ERROR:
        return "JSON parse error";

    case SERIAL_CONFIG_INVALID_ROOT:
        return "Invalid JSON structure";

    case SERIAL_CONFIG_INVALID_VALUE:
        return "Invalid serial configuration";

    case SERIAL_CONFIG_SERIALIZE_ERROR:
        return "JSON serialization error";

    case SERIAL_CONFIG_INVALID_ARGUMENT:
        return "Invalid argument";

    default:
        return "Unknown error";
    }
}
