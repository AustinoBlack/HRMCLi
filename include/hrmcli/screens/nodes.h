#ifndef HRMCLI_SCREEN_NODES_H
#define HRMCLI_SCREEN_NODES_H

#include "hrmcli/node.h"
#include "hrmcli/pane.h"

#define NODES_SCREEN_MAX_NODES 16

typedef enum {
    NODES_VIEW_LIST,
    NODES_VIEW_DETAIL
} NodesView;

typedef struct {
    Node nodes[NODES_SCREEN_MAX_NODES];

    int node_count;
    int selected;

    NodesView view;
    int load_failed;
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

/*
 * Returns 1 if Esc was consumed by the Nodes screen.
 *
 * Returns 0 if the Nodes screen is already at its
 * top level and the caller should navigate away.
 */
int screen_nodes_handle_escape(
    NodesScreenState *state
);

#endif
