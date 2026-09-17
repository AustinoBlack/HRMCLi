#ifndef HRMCLI_WORKSPACE_H
#define HRMCLI_WORKSPACE_H

#include "hrmcli/pane.h"
#include "hrmcli/ui.h"

typedef struct {
    UiRect rect;

    UiPane *primary;
    UiPane *secondary;

    int split_active;
    int focused_pane;
    int split_percent;
} UiWorkspace;

void ui_workspace_init(
    UiWorkspace *workspace,
    UiRect rect,
    UiPane *primary,
    UiPane *secondary
);

void ui_workspace_set_rect(
    UiWorkspace *workspace,
    UiRect rect
);

void ui_workspace_set_split(
    UiWorkspace *workspace,
    int enabled
);

void ui_workspace_toggle_split(
    UiWorkspace *workspace
);

void ui_workspace_toggle_focus(
    UiWorkspace *workspace
);

void ui_workspace_update_layout(
    UiWorkspace *workspace
);

void ui_workspace_draw(
    UiWorkspace *workspace
);

void ui_workspace_handle_key(
    UiWorkspace *workspace,
    int key
);

#endif
