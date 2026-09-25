#ifndef HRMCLI_SCREEN_LOGS_H
#define HRMCLI_SCREEN_LOGS_H

#include "hrmcli/pane.h"

void screen_logs_draw(
    UiPane *pane
);

void screen_logs_handle_key(
    UiPane *pane,
    int key
);

#endif
