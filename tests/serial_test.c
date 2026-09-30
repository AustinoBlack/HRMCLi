#include <stdio.h>

#include "hrmcli/serial.h"

int main(void)
{
    SerialDevice devices[SERIAL_MAX_DEVICES];
    int count = 0;

    if (
        serial_enumerate(
            devices,
            SERIAL_MAX_DEVICES,
            &count
        ) != 0
    ) {
        fprintf(
            stderr,
            "Failed to enumerate serial devices.\n"
        );

        return 1;
    }

    for (int i = 0; i < count; i++) {
        printf(
            "%-5s %-6s %-11s %s\n",
            serial_device_type_string(
                devices[i].type
            ),
            devices[i].stable_path
                ? "stable"
                : "direct",
            devices[i].available
                ? "available"
                : "unavailable",
            devices[i].path
        ); 
    }

    return 0;
}
