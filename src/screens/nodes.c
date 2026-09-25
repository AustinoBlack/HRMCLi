#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "hrmcli/screens/nodes.h"
#include "hrmcli/terminal.h"
#include "hrmcli/ui.h"


static void add_test_node(
    NodesScreenState *state,
    const char *name,
    const char *address,
    NodeProtocol protocol,
    NodeStatus status,
    NodePowerState power
)
{
    Node *node;

    if (
        state == NULL ||
        state->node_count >= NODES_SCREEN_MAX_NODES
    ) {
        return;
    }

    node = &state->nodes[state->node_count];

    snprintf(
        node->name,
        sizeof(node->name),
        "%s",
        name
    );

    snprintf(
        node->address,
        sizeof(node->address),
        "%s",
        address
    );

    node->protocol = protocol;
    node->status = status;
    node->power = power;

    state->node_count++;
}


void screen_nodes_init(
    NodesScreenState *state
)
{
    if (state == NULL) {
        return;
    }

    memset(
        state,
        0,
        sizeof(*state)
    );

    state->selected = 0;

    /*
     * Temporary in-memory test nodes.
     */
    add_test_node(
        state,
        "pve-Leela",
        "192.168.100.10",
        NODE_PROTOCOL_IPMI,
        NODE_STATUS_ONLINE,
        NODE_POWER_ON
    );

    add_test_node(
        state,
        "pve-Tycho",
        "192.168.100.11",
        NODE_PROTOCOL_IPMI,
        NODE_STATUS_OFFLINE,
        NODE_POWER_UNKNOWN
    );

    add_test_node(
        state,
        "pve-Thoth",
        "192.168.100.12",
        NODE_PROTOCOL_IPMI,
        NODE_STATUS_ONLINE,
        NODE_POWER_OFF
    );
}


void screen_nodes_draw(
    UiPane *pane,
    NodesScreenState *state
)
{
    UiRect content;

    char line[256];

    int row;

    if (
        pane == NULL ||
        state == NULL
    ) {
        return;
    }

    content = ui_rect_inset(
        pane->rect,
        2
    );

    if (
        content.width <= 0 ||
        content.height <= 0
    ) {
        return;
    }

    ui_draw_centered_text(
        content.row,
        content.col,
        content.width,
        "Managed Nodes"
    );

    if (state->node_count == 0) {
        ui_draw_centered_text(
            content.row + 3,
            content.col,
            content.width,
            "No nodes configured."
        );

        return;
    }

    row = content.row + 2;

    for (
        int i = 0;
        i < state->node_count;
        i++
    ) {
        Node *node;

        if (
            row >=
            content.row + content.height
        ) {
            break;
        }

        node = &state->nodes[i];

        snprintf(
            line,
            sizeof(line),
            "%-18s %-16s %-8s %-8s %-7s",
            node->name,
            node->address,
            node_protocol_string(
                node->protocol
            ),
            node_status_string(
                node->status
            ),
            node_power_string(
                node->power
            )
        );

        if (i == state->selected) {
            ui_set_reverse(1);
        }

        ui_draw_text(
            row,
            content.col,
            line
        );

        if (i == state->selected) {
            ui_set_reverse(0);
        }

        row++;
    }

    ui_draw_text(
        content.row + content.height - 1,
        content.col,
        "Up/Down Select   Home/End Jump   Enter Open   Esc Back"
    );
}


void screen_nodes_handle_key(
    NodesScreenState *state,
    int key
)
{
    if (
        state == NULL ||
        state->node_count <= 0
    ) {
        return;
    }

    switch (key) {
    case TERMINAL_KEY_UP:
        state->selected--;

        if (state->selected < 0) {
            state->selected =
                state->node_count - 1;
        }

        break;

    case TERMINAL_KEY_DOWN:
        state->selected++;

        if (
            state->selected >=
            state->node_count
        ) {
            state->selected = 0;
        }

        break;

    case TERMINAL_KEY_HOME:
        state->selected = 0;
        break;

    case TERMINAL_KEY_END:
        state->selected =
            state->node_count - 1;
        break;

    default:
        break;
    }
}
