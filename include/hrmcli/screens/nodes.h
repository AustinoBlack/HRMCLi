#ifndef HRMCLI_SCREEN_NODES_H
#define HRMCLI_SCREEN_NODES_H

#include "hrmcli/node.h"
#include "hrmcli/pane.h"

#define NODES_SCREEN_MAX_NODES 16
#define NODE_FORM_MESSAGE_MAX 128


typedef enum {
    NODES_VIEW_LIST,
    NODES_VIEW_DETAIL,
    NODES_VIEW_ADD
} NodesView;


typedef enum {
    NODE_FORM_NAME,
    NODE_FORM_ADDRESS,
    NODE_FORM_PROTOCOL,
    NODE_FORM_SAVE
} NodeFormField;


typedef struct {
    Node node;

    NodeFormField field;

    int cursor;

    char message[NODE_FORM_MESSAGE_MAX];
} NodeFormState;


typedef struct {
    Node nodes[NODES_SCREEN_MAX_NODES];

    int node_count;
    int selected;

    NodesView view;

    int load_failed;

    NodeFormState form;
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

int screen_nodes_handle_escape(
    NodesScreenState *state
);

#endif
