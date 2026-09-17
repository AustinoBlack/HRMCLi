#include <stddef.h>

#include "hrmcli/workspace.h"


void ui_workspace_init(
    UiWorkspace *workspace,
    UiRect rect,
    UiPane *primary,
    UiPane *secondary
)
{
    if (workspace == NULL) {
        return;
    }

    workspace->rect = rect;

    workspace->primary = primary;
    workspace->secondary = secondary;

    workspace->split_active = 0;
    workspace->focused_pane = 0;
    workspace->split_percent = 50;

    if (primary != NULL) {
        primary->visible = 1;

        ui_pane_set_focus(
            primary,
            1
        );
    }

    if (secondary != NULL) {
        secondary->visible = 0;

        ui_pane_set_focus(
            secondary,
            0
        );
    }

    ui_workspace_update_layout(workspace);
}


void ui_workspace_set_rect(
    UiWorkspace *workspace,
    UiRect rect
)
{
    if (workspace == NULL) {
        return;
    }

    workspace->rect = rect;

    ui_workspace_update_layout(workspace);
}


void ui_workspace_set_split(
    UiWorkspace *workspace,
    int enabled
)
{
    if (workspace == NULL) {
        return;
    }

    if (
        workspace->secondary == NULL ||
        !workspace->secondary->focusable
    ) {
        workspace->split_active = 0;
        workspace->focused_pane = 0;
    } else {
        workspace->split_active =
            enabled ? 1 : 0;
    }

    if (!workspace->split_active) {
        workspace->focused_pane = 0;
    }

    ui_workspace_update_layout(workspace);
}


void ui_workspace_toggle_split(
    UiWorkspace *workspace
)
{
    if (workspace == NULL) {
        return;
    }

    ui_workspace_set_split(
        workspace,
        !workspace->split_active
    );
}


void ui_workspace_toggle_focus(
    UiWorkspace *workspace
)
{
    if (
        workspace == NULL ||
        !workspace->split_active ||
        workspace->secondary == NULL
    ) {
        return;
    }

    workspace->focused_pane =
        workspace->focused_pane == 0 ? 1 : 0;

    ui_pane_set_focus(
        workspace->primary,
        workspace->focused_pane == 0
    );

    ui_pane_set_focus(
        workspace->secondary,
        workspace->focused_pane == 1
    );
}


void ui_workspace_update_layout(
    UiWorkspace *workspace
)
{
    UiRect left;
    UiRect right;

    if (
        workspace == NULL ||
        workspace->primary == NULL
    ) {
        return;
    }

    if (
        !workspace->split_active ||
        workspace->secondary == NULL
    ) {
        ui_pane_set_rect(
            workspace->primary,
            workspace->rect
        );

        workspace->primary->visible = 1;

        ui_pane_set_focus(
            workspace->primary,
            1
        );

        workspace->focused_pane = 0;

        if (workspace->secondary != NULL) {
            workspace->secondary->visible = 0;

            ui_pane_set_focus(
                workspace->secondary,
                0
            );
        }

        return;
    }

    ui_split_vertical(
        workspace->rect,
        workspace->split_percent,
        &left,
        &right
    );

    ui_pane_set_rect(
        workspace->primary,
        left
    );

    ui_pane_set_rect(
        workspace->secondary,
        right
    );

    workspace->primary->visible = 1;
    workspace->secondary->visible = 1;

    ui_pane_set_focus(
        workspace->primary,
        workspace->focused_pane == 0
    );

    ui_pane_set_focus(
        workspace->secondary,
        workspace->focused_pane == 1
    );
}


void ui_workspace_draw(
    UiWorkspace *workspace
)
{
    if (workspace == NULL) {
        return;
    }

    if (workspace->primary != NULL) {
        ui_pane_draw(
            workspace->primary
        );
    }

    if (
        workspace->split_active &&
        workspace->secondary != NULL
    ) {
        ui_pane_draw(
            workspace->secondary
        );
    }
}


void ui_workspace_handle_key(
    UiWorkspace *workspace,
    int key
)
{
    if (
        workspace == NULL ||
        workspace->primary == NULL
    ) {
        return;
    }

    if (
        workspace->split_active &&
        workspace->focused_pane == 1 &&
        workspace->secondary != NULL
    ) {
        ui_pane_handle_key(
            workspace->secondary,
            key
        );

        return;
    }

    ui_pane_handle_key(
        workspace->primary,
        key
    );
}
