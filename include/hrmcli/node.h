#ifndef HRMCLI_NODE_H
#define HRMCLI_NODE_H

#define NODE_NAME_MAX 64
#define NODE_ADDRESS_MAX 64

typedef enum {
    NODE_PROTOCOL_IPMI,
    NODE_PROTOCOL_REDFISH
} NodeProtocol;

typedef enum {
    NODE_STATUS_UNKNOWN,
    NODE_STATUS_OFFLINE,
    NODE_STATUS_ONLINE
} NodeStatus;

typedef enum {
    NODE_POWER_UNKNOWN,
    NODE_POWER_OFF,
    NODE_POWER_ON
} NodePowerState;

typedef struct {
    char name[NODE_NAME_MAX];
    char address[NODE_ADDRESS_MAX];

    NodeProtocol protocol;
    NodeStatus status;
    NodePowerState power;
} Node;

const char *node_protocol_string(
    NodeProtocol protocol
);

const char *node_status_string(
    NodeStatus status
);

const char *node_power_string(
    NodePowerState power
);

#endif
