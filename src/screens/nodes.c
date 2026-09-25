#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "hrmcli/screens/nodes.h"
#include "hrmcli/node_config.h"
#include "hrmcli/terminal.h"
#include "hrmcli/ui.h"

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
    state->view = NODES_VIEW_LIST;

    if (
        node_config_load(
            "config/defaults/nodes.json",
            state->nodes,
            NODES_SCREEN_MAX_NODES,
            &state->node_count
        ) != 0
    ) {
        state->load_failed = 1;
        state->node_count = 0;
        return;
    }

    state->load_failed = 0;
}


static void draw_node_list(
    UiPane *pane,
    NodesScreenState *state
)
{
    UiRect content;

    char line[256];

    int row;

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

    /* Failed to load JSON trigger */
    if (state->load_failed) {
        ui_draw_centered_text(
            content.row + 3,
            content.col,
            content.width,
            "Failed to load node configuration."
        );
        return;
    }

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
            content.row + content.height - 1
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


static void draw_node_detail(
    UiPane *pane,
    NodesScreenState *state
)
{
    UiRect content;

    Node *node;

    char line[256];

    int row;

    if (
        state->node_count <= 0 ||
        state->selected < 0 ||
        state->selected >= state->node_count
    ) {
        return;
    }

    node = &state->nodes[state->selected];

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
        node->name
    );

    row = content.row + 3;

    snprintf(
        line,
        sizeof(line),
        "Name:        %s",
        node->name
    );

    ui_draw_text(
        row++,
        content.col + 2,
        line
    );

    snprintf(
        line,
        sizeof(line),
        "BMC Address: %s",
        node->address
    );

    ui_draw_text(
        row++,
        content.col + 2,
        line
    );

    snprintf(
        line,
        sizeof(line),
        "Protocol:    %s",
        node_protocol_string(
            node->protocol
        )
    );

    ui_draw_text(
        row++,
        content.col + 2,
        line
    );

    snprintf(
        line,
        sizeof(line),
        "BMC Status:  %s",
        node_status_string(
            node->status
        )
    );

    ui_draw_text(
        row++,
        content.col + 2,
        line
    );

    snprintf(
        line,
        sizeof(line),
        "Power State: %s",
        node_power_string(
            node->power
        )
    );

    ui_draw_text(
        row++,
        content.col + 2,
        line
    );

    /*
     * These are intentionally non-functional
     * placeholders for upcoming node actions.
     */
    row += 2;

    ui_draw_text(
        row,
        content.col + 2,
        "[ Power ]   [ Sensors ]   [ SEL ]   [ Info ]"
    );

    ui_draw_text(
        content.row + content.height - 1,
        content.col,
        "Esc Back to Nodes"
    );
}


void screen_nodes_draw(
    UiPane *pane,
    NodesScreenState *state
)
{
    if (
        pane == NULL ||
        state == NULL
    ) {
        return;
    }

    switch (state->view) {
    case NODES_VIEW_LIST:
        draw_node_list(
            pane,
            state
        );
        break;

    case NODES_VIEW_DETAIL:
        draw_node_detail(
            pane,
            state
        );
        break;
    }
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

    /*
     * Detail view does not yet have
     * interactive controls.
     */
    if (state->view == NODES_VIEW_DETAIL) {
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

    case TERMINAL_KEY_ENTER:
        state->view =
            NODES_VIEW_DETAIL;
        break;

    default:
        break;
    }
}


int screen_nodes_handle_escape(
    NodesScreenState *state
)
{
    if (state == NULL) {
        return 0;
    }

    if (
        state->view ==
        NODES_VIEW_DETAIL
    ) {
        state->view =
            NODES_VIEW_LIST;

        return 1;
    }

    return 0;
}
