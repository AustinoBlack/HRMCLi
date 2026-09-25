#ifndef HRMCLI_SCREEN_DASHBOARD_H
#define HRMCLI_SCREEN_DASHBOARD_H

#include "hrmcli/pane.h"

void screen_dashboard_draw(
    UiPane *pane
);

void screen_dashboard_handle_key(
    UiPane *pane,
    int key
);

#endif
