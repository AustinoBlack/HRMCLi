#ifndef HRMCLI_NODE_CONFIG_H
#define HRMCLI_NODE_CONFIG_H

#include "hrmcli/node.h"

int node_config_load(
    const char *path,
    Node *nodes,
    int max_nodes,
    int *node_count
);

#endif
