#ifndef HRMCLI_SCREEN_NODES_H
#define HRMCLI_SCREEN_NODES_H

#include "hrmcli/pane.h"

void screen_nodes_draw(
    UiPane *pane
);

void screen_nodes_handle_key(
    UiPane *pane,
    int key
);

#endif
