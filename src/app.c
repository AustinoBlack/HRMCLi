#include <stdio.h>

#include "hrmcli/app.h"
#include "hrmcli/config.h"
#include "hrmcli/log.h"
#include "hrmcli/tui.h"

int app_run(void)
{
    if (config_init() != 0) {
        fprintf(stderr, "Failed to initialize configuration.\n");
        return 1;
    }

    if (log_init() != 0) {
        fprintf(stderr, "Failed to initialize logging.\n");
        return 1;
    }

    return tui_run();
}
