#include <stdio.h>
#include <string.h>

#include "hrmcli/node_config.h"
#include "hrmcli/paths.h"
#include "hrmcli/screens/nodes.h"
#include "hrmcli/terminal.h"
#include "hrmcli/ui.h"

static void handle_text_field(
    char *buffer,
    int buffer_size,
    int *cursor,
    int key
);

static void handle_add_node_key(
    NodesScreenState *state,
    int key
);

static void save_add_node(
    NodesScreenState *state
);

static void reset_node_form(
    NodesScreenState *state
)
{
    if (state == NULL) {
        return;
    }

    memset(
        &state->form,
        0,
        sizeof(state->form)
    );

    state->form.node.protocol =
        NODE_PROTOCOL_IPMI;

    state->form.node.status =
        NODE_STATUS_UNKNOWN;

    state->form.node.power =
        NODE_POWER_UNKNOWN;

    state->form.field =
        NODE_FORM_NAME;

    state->form.cursor = 0;
}


void screen_nodes_init(
    NodesScreenState *state
)
{
    NodeConfigStatus status;

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

    status = node_config_load(
        hrmcli_nodes_path(),
        state->nodes,
        NODES_SCREEN_MAX_NODES,
        &state->node_count
    );

    switch (status) {
    case NODE_CONFIG_OK:
    case NODE_CONFIG_INVALID_NODE:
    case NODE_CONFIG_TOO_MANY_NODES:
        /*
         * Configuration contained usable node data.
         */
        state->load_failed = 0;
        break;

    default:
        /*
         * Configuration could not be used.
         */
        state->node_count = 0;
        state->load_failed = 1;
        break;
    }
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

    /*
     * Failed to load JSON.
     */
    if (state->load_failed) {
        ui_draw_centered_text(
            content.row + 3,
            content.col,
            content.width,
            "Failed to load node configuration."
        );

        ui_draw_text(
            content.row + content.height - 1,
            content.col,
            "A Add Node   Esc Back"
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

        ui_draw_text(
            content.row + content.height - 1,
            content.col,
            "A Add Node   Esc Back"
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
        "Up/Down Select   Home/End Jump   Enter Open   A Add   Esc Back"
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

static void draw_add_node(
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
        "Add Node"
    );

    row = content.row + 3;

    /*
     * Name
     */
    snprintf(
        line,
        sizeof(line),
        "Name:        %s",
        state->form.node.name
    );

    if (
        state->form.field ==
        NODE_FORM_NAME
    ) {
        ui_set_reverse(1);
    }

    ui_draw_text(
        row++,
        content.col + 2,
        line
    );

    if (
        state->form.field ==
        NODE_FORM_NAME
    ) {
        ui_set_reverse(0);
    }

    /*
     * BMC address
     */
    snprintf(
        line,
        sizeof(line),
        "BMC Address: %s",
        state->form.node.address
    );

    if (
        state->form.field ==
        NODE_FORM_ADDRESS
    ) {
        ui_set_reverse(1);
    }

    ui_draw_text(
        row++,
        content.col + 2,
        line
    );

    if (
        state->form.field ==
        NODE_FORM_ADDRESS
    ) {
        ui_set_reverse(0);
    }

    /*
     * Protocol
     */
    snprintf(
        line,
        sizeof(line),
        "Protocol:    %s",
        node_protocol_string(
            state->form.node.protocol
        )
    );

    if (
        state->form.field ==
        NODE_FORM_PROTOCOL
    ) {
        ui_set_reverse(1);
    }

    ui_draw_text(
        row++,
        content.col + 2,
        line
    );

    if (
        state->form.field ==
        NODE_FORM_PROTOCOL
    ) {
        ui_set_reverse(0);
    }

    /*
     * Save
     */
    row += 2;

    if (
        state->form.field ==
        NODE_FORM_SAVE
    ) {
        ui_set_reverse(1);
    }

    ui_draw_text(
        row,
        content.col + 2,
        "[ Save ]"
    );

    if (
        state->form.field ==
        NODE_FORM_SAVE
    ) {
        ui_set_reverse(0);
    }

    if (
        state->form.message[0] != '\0'
    ) {
        ui_draw_text(
            row + 2,
            content.col + 2,
            state->form.message
        );
    }

    ui_draw_text(
        content.row + content.height - 1,
        content.col,
        "Tab Next Field   Left/Right Protocol   Enter Save   Esc Cancel"
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

    case NODES_VIEW_ADD:
        draw_add_node(
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
    if (state == NULL) {
        return;
    }

    /*
     * Enter Add Node mode from the list.
     *
     * This intentionally happens before checking
     * node_count so users can add the first node.
     */
    if (
        state->view == NODES_VIEW_LIST &&
        (key == 'a' || key == 'A')
    ) {
        reset_node_form(
            state
        );

        state->view =
            NODES_VIEW_ADD;

        return;
    }

    /*
     * Add Node does not have editable controls yet.
     * Those will be implemented next.
     */
    if (state->view == NODES_VIEW_ADD) {
        handle_add_node_key(
            state,
            key
        );

        return;
    }

    /*
     * Detail view does not yet have
     * interactive controls.
     */
    if (state->view == NODES_VIEW_DETAIL) {
        return;
    }

    /*
     * List navigation requires at least one node.
     */
    if (state->node_count <= 0) {
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

    if (
        state->view ==
        NODES_VIEW_ADD
    ) {
        state->view =
            NODES_VIEW_LIST;

        return 1;
    }

    return 0;
}

static void handle_text_field(
    char *buffer,
    int buffer_size,
    int *cursor,
    int key
)
{
    int length;

    if (
        buffer == NULL ||
        cursor == NULL ||
        buffer_size <= 0
    ) {
        return;
    }

    length = (int)strlen(buffer);

    if (key == TERMINAL_KEY_BACKSPACE) {
        if (*cursor > 0) {
            memmove(
                &buffer[*cursor - 1],
                &buffer[*cursor],
                (size_t)(length - *cursor + 1)
            );

            (*cursor)--;
        }

        return;
    }

    if (
        key >= 32 &&
        key <= 126 &&
        length < buffer_size - 1
    ) {
        memmove(
            &buffer[*cursor + 1],
            &buffer[*cursor],
            (size_t)(length - *cursor + 1)
        );

        buffer[*cursor] = (char)key;

        (*cursor)++;
    }
}

static void handle_add_node_key(
    NodesScreenState *state,
    int key
)
{
    if (state == NULL) {
        return;
    }

    /*
     * Tab always advances to the next field.
     */
    if (key == TERMINAL_KEY_TAB) {
        switch (state->form.field) {
        case NODE_FORM_NAME:
            state->form.field =
                NODE_FORM_ADDRESS;

            state->form.cursor =
                (int)strlen(
                    state->form.node.address
                );
            break;

        case NODE_FORM_ADDRESS:
            state->form.field =
                NODE_FORM_PROTOCOL;

            state->form.cursor = 0;
            break;

        case NODE_FORM_PROTOCOL:
            state->form.field =
                NODE_FORM_SAVE;

            state->form.cursor = 0;
            break;

        case NODE_FORM_SAVE:
            state->form.field =
                NODE_FORM_NAME;

            state->form.cursor =
                (int)strlen(
                    state->form.node.name
                );
            break;
        }

        return;
    }

    /*
     * Handle input for whichever field
     * currently has focus.
     */
    switch (state->form.field) {
    case NODE_FORM_NAME:
        handle_text_field(
            state->form.node.name,
            NODE_NAME_MAX,
            &state->form.cursor,
            key
        );
        break;

    case NODE_FORM_ADDRESS:
        handle_text_field(
            state->form.node.address,
            NODE_ADDRESS_MAX,
            &state->form.cursor,
            key
        );
        break;

    case NODE_FORM_PROTOCOL:
        if (
            key == TERMINAL_KEY_LEFT ||
            key == TERMINAL_KEY_RIGHT
        ) {
            if (
                state->form.node.protocol ==
                NODE_PROTOCOL_IPMI
            ) {
                state->form.node.protocol =
                    NODE_PROTOCOL_REDFISH;
            } else {
                state->form.node.protocol =
                    NODE_PROTOCOL_IPMI;
            }
        }

        break;

    case NODE_FORM_SAVE:
        if (key == TERMINAL_KEY_ENTER) {
            save_add_node(
                state
            );
        }

        break;
    }
}

static void save_add_node(
    NodesScreenState *state
)
{
    NodeConfigStatus status;

    if (state == NULL) {
        return;
    }

    /*
     * Basic validation.
     */
    if (state->form.node.name[0] == '\0') {
        snprintf(
            state->form.message,
            sizeof(state->form.message),
            "Node name is required."
        );

        return;
    }

    if (state->form.node.address[0] == '\0') {
        snprintf(
            state->form.message,
            sizeof(state->form.message),
            "BMC address is required."
        );

        return;
    }

    if (
        state->node_count >=
        NODES_SCREEN_MAX_NODES
    ) {
        snprintf(
            state->form.message,
            sizeof(state->form.message),
            "Maximum node count reached."
        );

        return;
    }

    /*
     * Add the temporary form node to the
     * in-memory node list.
     */
    state->nodes[state->node_count] =
        state->form.node;

    state->node_count++;

    /*
     * Persist the complete node list.
     */
    status = node_config_save(
        hrmcli_nodes_path(),
        state->nodes,
        state->node_count
    );

    if (status != NODE_CONFIG_OK) {
        /*
         * Roll back the in-memory append if
         * persistence failed.
         */
        state->node_count--;

        memset(
            &state->nodes[state->node_count],
            0,
            sizeof(state->nodes[state->node_count])
        );

        snprintf(
            state->form.message,
            sizeof(state->form.message),
            "Save failed: %s",
            node_config_status_string(status)
        );

        return;
    }

    /*
     * Select the newly-created node and return
     * to the list.
     */
    state->selected =
        state->node_count - 1;

    state->load_failed = 0;

    state->view =
        NODES_VIEW_LIST;
}
