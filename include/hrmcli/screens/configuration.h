#ifndef HRMCLI_SCREEN_CONFIGURATION_H
#define HRMCLI_SCREEN_CONFIGURATION_H

#include "hrmcli/pane.h"

void screen_configuration_draw(
    UiPane *pane
);

void screen_configuration_handle_key(
    UiPane *pane,
    int key
);

#endif
