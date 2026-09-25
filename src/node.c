#include "hrmcli/node.h"

const char *node_protocol_string(
    NodeProtocol protocol
)
{
    switch (protocol) {
    case NODE_PROTOCOL_IPMI:
        return "IPMI";

    case NODE_PROTOCOL_REDFISH:
        return "Redfish";

    default:
        return "Unknown";
    }
}


const char *node_status_string(
    NodeStatus status
)
{
    switch (status) {
    case NODE_STATUS_ONLINE:
        return "ONLINE";

    case NODE_STATUS_OFFLINE:
        return "OFFLINE";

    case NODE_STATUS_UNKNOWN:
    default:
        return "UNKNOWN";
    }
}


const char *node_power_string(
    NodePowerState power
)
{
    switch (power) {
    case NODE_POWER_ON:
        return "ON";

    case NODE_POWER_OFF:
        return "OFF";

    case NODE_POWER_UNKNOWN:
    default:
        return "UNKNOWN";
    }
}
