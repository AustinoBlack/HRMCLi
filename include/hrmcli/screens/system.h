#ifndef HRMCLI_SCREEN_SYSTEM_H
#define HRMCLI_SCREEN_SYSTEM_H

#include "hrmcli/pane.h"

void screen_system_draw(
    UiPane *pane
);

void screen_system_handle_key(
    UiPane *pane,
    int key
);

#endif
