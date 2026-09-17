#include <stddef.h>

#include "hrmcli/menu.h"
#include "hrmcli/pane.h"
#include "hrmcli/terminal.h"
#include "hrmcli/tui.h"
#include "hrmcli/ui.h"
#include "hrmcli/workspace.h"


typedef enum {
    SCREEN_MAIN_MENU,
    SCREEN_DASHBOARD,
    SCREEN_NODES,
    SCREEN_CONFIGURATION,
    SCREEN_LOGS,
    SCREEN_CLI,
    SCREEN_SYSTEM
} TuiScreen;


typedef struct {
    TuiScreen current_screen;
    UiMenu main_menu;
    int quit_requested;
} TuiState;


static const char *main_menu_items[] = {
    "Dashboard",
    "Nodes",
    "Configuration",
    "Logs",
    "HRMCLi CLI",
    "System",
    "Quit"
};


#define MAIN_MENU_COUNT \
    ((int)(sizeof(main_menu_items) / sizeof(main_menu_items[0])))


static void set_screen(
    UiPane *pane,
    TuiState *state,
    TuiScreen screen
)
{
    if (
        pane == NULL ||
        state == NULL
    ) {
        return;
    }

    state->current_screen = screen;

    switch (screen) {
    case SCREEN_MAIN_MENU:
        pane->title = "HRMCLI";
        break;

    case SCREEN_DASHBOARD:
        pane->title = "DASHBOARD";
        break;

    case SCREEN_NODES:
        pane->title = "NODES";
        break;

    case SCREEN_CONFIGURATION:
        pane->title = "CONFIGURATION";
        break;

    case SCREEN_LOGS:
        pane->title = "LOGS";
        break;

    case SCREEN_CLI:
        pane->title = "HRMCLI CLI";
        break;

    case SCREEN_SYSTEM:
        pane->title = "SYSTEM";
        break;
    }
}


static void draw_placeholder_screen(
    UiPane *pane,
    const char *heading
)
{
    UiRect content;

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
        content.row + 2,
        content.col,
        content.width,
        heading
    );

    ui_draw_centered_text(
        content.row + 4,
        content.col,
        content.width,
        "This screen is not implemented yet."
    );

    ui_draw_centered_text(
        content.row + 6,
        content.col,
        content.width,
        "Press Esc to return."
    );
}


static void draw_nodes_screen(
    UiPane *pane
)
{
    UiRect content;

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
        content.row + 2,
        content.col,
        content.width,
        "Managed Nodes"
    );

    ui_draw_centered_text(
        content.row + 4,
        content.col,
        content.width,
        "No nodes configured."
    );

    ui_draw_centered_text(
        content.row + 6,
        content.col,
        content.width,
        "Press Esc to return."
    );
}


static void draw_main_menu(
    UiPane *pane,
    TuiState *state
)
{
    UiRect content;

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

    ui_menu_set_rect(
        &state->main_menu,
        content
    );

    ui_menu_draw(
        &state->main_menu
    );
}


static void draw_primary_pane(
    UiPane *pane
)
{
    TuiState *state;

    if (pane == NULL) {
        return;
    }

    state = pane->userdata;

    if (state == NULL) {
        return;
    }

    switch (state->current_screen) {
    case SCREEN_MAIN_MENU:
        draw_main_menu(
            pane,
            state
        );
        break;

    case SCREEN_DASHBOARD:
        draw_placeholder_screen(
            pane,
            "Dashboard"
        );
        break;

    case SCREEN_NODES:
        draw_nodes_screen(
            pane
        );
        break;

    case SCREEN_CONFIGURATION:
        draw_placeholder_screen(
            pane,
            "Configuration"
        );
        break;

    case SCREEN_LOGS:
        draw_placeholder_screen(
            pane,
            "Logs"
        );
        break;

    case SCREEN_CLI:
        draw_placeholder_screen(
            pane,
            "HRMCLi Command Line"
        );
        break;

    case SCREEN_SYSTEM:
        draw_placeholder_screen(
            pane,
            "System"
        );
        break;
    }
}


static void draw_secondary_pane(
    UiPane *pane
)
{
    UiRect content;

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
        content.row + 2,
        content.col,
        content.width,
        "HRMCLi CLI"
    );

    ui_draw_centered_text(
        content.row + 4,
        content.col,
        content.width,
        "HRMCLi> _"
    );
}


static void handle_main_menu_selection(
    UiPane *pane,
    TuiState *state,
    int selected
)
{
    switch (selected) {
    case 0:
        set_screen(
            pane,
            state,
            SCREEN_DASHBOARD
        );
        break;

    case 1:
        set_screen(
            pane,
            state,
            SCREEN_NODES
        );
        break;

    case 2:
        set_screen(
            pane,
            state,
            SCREEN_CONFIGURATION
        );
        break;

    case 3:
        set_screen(
            pane,
            state,
            SCREEN_LOGS
        );
        break;

    case 4:
        set_screen(
            pane,
            state,
            SCREEN_CLI
        );
        break;

    case 5:
        set_screen(
            pane,
            state,
            SCREEN_SYSTEM
        );
        break;

    case 6:
        state->quit_requested = 1;
        break;

    default:
        break;
    }
}


static void handle_primary_pane_key(
    UiPane *pane,
    int key
)
{
    TuiState *state;
    int selected;

    if (pane == NULL) {
        return;
    }

    state = pane->userdata;

    if (state == NULL) {
        return;
    }

    /*
     * Currently the main menu is the only
     * primary screen with local key handling.
     */
    if (
        state->current_screen !=
        SCREEN_MAIN_MENU
    ) {
        return;
    }

    selected = ui_menu_handle_key(
        &state->main_menu,
        key
    );

    if (
        selected !=
        UI_MENU_NO_SELECTION
    ) {
        handle_main_menu_selection(
            pane,
            state,
            selected
        );
    }
}


static void draw_screen(
    UiWorkspace *workspace
)
{
    terminal_clear();

    ui_workspace_draw(
        workspace
    );
}


int tui_run(void)
{
    UiRect empty_rect = {0};
    UiRect screen;
    UiRect content;

    UiPane primary;
    UiPane secondary;

    UiWorkspace workspace;

    TuiState state;

    int key;
    int running = 1;

    if (terminal_init() != 0) {
        return 1;
    }

    /*
     * Initialize application TUI state.
     */
    state.current_screen =
        SCREEN_MAIN_MENU;

    state.quit_requested = 0;

    ui_menu_init(
        &state.main_menu,
        empty_rect,
        main_menu_items,
        MAIN_MENU_COUNT
    );

    /*
     * Initialize primary and secondary panes.
     */
    ui_pane_init(
        &primary,
        empty_rect,
        "HRMCLI"
    );

    ui_pane_init(
        &secondary,
        empty_rect,
        "HRMCLI CLI"
    );

    primary.userdata = &state;
    primary.draw = draw_primary_pane;
    primary.handle_key =
        handle_primary_pane_key;

    secondary.draw =
        draw_secondary_pane;

    /*
     * Determine initial workspace dimensions.
     */
    screen = ui_get_screen_rect();

    content = ui_rect_inset(
        screen,
        1
    );

    ui_workspace_init(
        &workspace,
        content,
        &primary,
        &secondary
    );

    draw_screen(
        &workspace
    );

    /*
     * Main event loop.
     */
    while (running) {
        key = terminal_read_key();

        if (key < 0) {
            break;
        }

        switch (key) {
        case TERMINAL_KEY_RESIZE:
            screen =
                ui_get_screen_rect();

            content =
                ui_rect_inset(
                    screen,
                    1
                );

            ui_workspace_set_rect(
                &workspace,
                content
            );

            draw_screen(
                &workspace
            );

            continue;

        case TERMINAL_KEY_TAB:
            ui_workspace_toggle_focus(
                &workspace
            );

            draw_screen(
                &workspace
            );

            continue;

        case TERMINAL_KEY_ESCAPE:
            /*
             * Esc returns to the main menu
             * from any primary screen.
             */
            if (
                state.current_screen !=
                SCREEN_MAIN_MENU
            ) {
                set_screen(
                    &primary,
                    &state,
                    SCREEN_MAIN_MENU
                );

                draw_screen(
                    &workspace
                );
            }

            continue;

        case TERMINAL_KEY_TERMINATE:
        case TERMINAL_KEY_CTRL_C:
            running = 0;
            continue;

        default:
            break;
        }

        /*
         * Temporary split-view shortcut.
         */
        if (
            key == 's' ||
            key == 'S'
        ) {
            ui_workspace_toggle_split(
                &workspace
            );

            draw_screen(
                &workspace
            );

            continue;
        }

        /*
         * Global quit shortcut.
         */
        if (
            key == 'q' ||
            key == 'Q'
        ) {
            running = 0;
            continue;
        }

        /*
         * Forward input to whichever pane
         * currently owns focus.
         */
        ui_workspace_handle_key(
            &workspace,
            key
        );

        if (state.quit_requested) {
            running = 0;
            continue;
        }

        draw_screen(
            &workspace
        );
    }

    terminal_clear();
    terminal_move_cursor(1, 1);
    terminal_shutdown();

    return 0;
}
