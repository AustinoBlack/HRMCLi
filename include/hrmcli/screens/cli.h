#ifndef HRMCLI_SCREEN_CLI_H
#define HRMCLI_SCREEN_CLI_H

#include "hrmcli/pane.h"

void screen_cli_draw(
    UiPane *pane
);

void screen_cli_handle_key(
    UiPane *pane,
    int key
);

#endif
