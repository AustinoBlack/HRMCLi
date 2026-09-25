#ifndef HRMCLI_SCREEN_NODES_H
#define HRMCLI_SCREEN_NODES_H

#include "hrmcli/node.h"
#include "hrmcli/pane.h"

#define NODES_SCREEN_MAX_NODES 16

typedef struct {
    Node nodes[NODES_SCREEN_MAX_NODES];

    int node_count;
    int selected;
} NodesScreenState;

void screen_nodes_init(
    NodesScreenState *state
);

void screen_nodes_draw(
    UiPane *pane,
    NodesScreenState *state
);

void screen_nodes_handle_key(
    NodesScreenState *state,
    int key
);

#endif
