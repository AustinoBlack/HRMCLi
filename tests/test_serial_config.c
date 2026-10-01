#include <stdio.h>

#include "hrmcli/paths.h"
#include "hrmcli/serial.h"

/*
 * This tests default serial config for serial.json 
 */

int main(void)
{
    SerialConfig config;
    SerialConfigStatus status;

    serial_config_defaults(
        &config
    );

    status = serial_config_load(
        hrmcli_serial_config_path(),
        &config
    );

    printf(
        "load: %s\n",
        serial_config_status_string(status)
    );

    printf("device: %s\n", config.device);
    printf("baud: %d\n", config.baud);
    printf("data bits: %d\n", config.data_bits);
    printf("stop bits: %d\n", config.stop_bits);

    return 0;
}
