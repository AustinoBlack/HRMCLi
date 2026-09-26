#ifndef HRMCLI_NODE_CONFIG_H
#define HRMCLI_NODE_CONFIG_H

#include "hrmcli/node.h"

typedef enum {
    NODE_CONFIG_OK = 0,

    NODE_CONFIG_FILE_NOT_FOUND,
    NODE_CONFIG_FILE_READ_ERROR,
    NODE_CONFIG_FILE_WRITE_ERROR,

    NODE_CONFIG_PARSE_ERROR,
    NODE_CONFIG_INVALID_ROOT,
    NODE_CONFIG_INVALID_NODE,
    NODE_CONFIG_TOO_MANY_NODES,
    NODE_CONFIG_SERIALIZE_ERROR,

    NODE_CONFIG_INVALID_ARGUMENT
} NodeConfigStatus;

NodeConfigStatus node_config_load(
    const char *path,
    Node *nodes,
    int max_nodes,
    int *node_count
);

NodeConfigStatus node_config_save(
    const char *path,
    const Node *nodes,
    int node_count
);

const char *node_config_status_string(
    NodeConfigStatus status
);

#endif
