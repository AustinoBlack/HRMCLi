#define _XOPEN_SOURCE 700

#include <glob.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "hrmcli/serial.h"

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

static int serial_device_is_available(
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

    device->available =
    serial_device_is_available(
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
