#include <stddef.h>
#include <time.h>

#include "hrmcli/menu.h"
#include "hrmcli/pane.h"
#include "hrmcli/startup.h"

#include "hrmcli/screens/startup.h"
#include "hrmcli/screens/main_menu.h"
#include "hrmcli/screens/dashboard.h"
#include "hrmcli/screens/nodes.h"
#include "hrmcli/screens/configuration.h"
#include "hrmcli/screens/logs.h"
#include "hrmcli/screens/cli.h"
#include "hrmcli/screens/system.h"

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
    NodesScreenState nodes;

    int quit_requested;
} TuiState;


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


static void draw_startup(
    UiPane *pane,
    StartupState *state
)
{
    terminal_clear();

    ui_pane_draw_frame(
        pane
    );

    screen_startup_draw(
        pane,
        state
    );
}


static void show_startup_screen(void)
{
    UiRect screen;
    UiRect content;

    UiPane startup_pane;

    StartupState startup_state;

    struct timespec delay;

    /*
     * Run the actual startup checks.
     */
    startup_init(
        &startup_state
    );

    startup_run_checks(
        &startup_state
    );

    /*
     * Give the startup screen nearly the
     * entire terminal.
     */
    screen = ui_get_screen_rect();

    content = ui_rect_inset(
        screen,
        1
    );

    ui_pane_init(
        &startup_pane,
        content,
        "STARTUP"
    );

    startup_pane.focusable = 0;

    draw_startup(
        &startup_pane,
        &startup_state
    );

    /*
     * Keep the completed checklist visible
     * briefly before entering the main UI.
     *
     * We can make this configurable later.
     */
    delay.tv_sec = 3;
    delay.tv_nsec = 250000000L;

    nanosleep(
        &delay,
        NULL
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
        screen_main_menu_draw(
            pane,
            &state->main_menu
        );
        break;

    case SCREEN_DASHBOARD:
        screen_dashboard_draw(
            pane
        );
        break;

    case SCREEN_NODES:
        screen_nodes_draw(
            pane,
            &state->nodes
        );
        break;

    case SCREEN_CONFIGURATION:
        screen_configuration_draw(
            pane
        );
        break;

    case SCREEN_LOGS:
        screen_logs_draw(
            pane
        );
        break;

    case SCREEN_CLI:
        screen_cli_draw(
            pane
        );
        break;

    case SCREEN_SYSTEM:
        screen_system_draw(
            pane
        );
        break;
    }
}


static void handle_main_menu_selection(
    UiPane *pane,
    TuiState *state,
    int selected
)
{
    if (
        pane == NULL ||
        state == NULL
    ) {
        return;
    }

    switch (selected) {
    case MAIN_MENU_DASHBOARD:
        set_screen(
            pane,
            state,
            SCREEN_DASHBOARD
        );
        break;

    case MAIN_MENU_NODES:
        set_screen(
            pane,
            state,
            SCREEN_NODES
        );
        break;

    case MAIN_MENU_CONFIGURATION:
        set_screen(
            pane,
            state,
            SCREEN_CONFIGURATION
        );
        break;

    case MAIN_MENU_LOGS:
        set_screen(
            pane,
            state,
            SCREEN_LOGS
        );
        break;

    case MAIN_MENU_CLI:
        set_screen(
            pane,
            state,
            SCREEN_CLI
        );
        break;

    case MAIN_MENU_SYSTEM:
        set_screen(
            pane,
            state,
            SCREEN_SYSTEM
        );
        break;

    case MAIN_MENU_QUIT:
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

    switch (state->current_screen) {
    case SCREEN_MAIN_MENU:
        selected =
            screen_main_menu_handle_key(
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

        break;

    case SCREEN_DASHBOARD:
        screen_dashboard_handle_key(
            pane,
            key
        );
        break;

    case SCREEN_NODES:
        screen_nodes_handle_key(
            &state->nodes,
            key
        );
        break;

    case SCREEN_CONFIGURATION:
        screen_configuration_handle_key(
            pane,
            key
        );
        break;

    case SCREEN_LOGS:
        screen_logs_handle_key(
            pane,
            key
        );
        break;

    case SCREEN_CLI:
        screen_cli_handle_key(
            pane,
            key
        );
        break;

    case SCREEN_SYSTEM:
        screen_system_handle_key(
            pane,
            key
        );
        break;
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

    /*
     * We cannot show anything until the
     * terminal backend has initialized.
     */
    if (terminal_init() != 0) {
        return 1;
    }

    /*
     * Run and display startup checks before
     * initializing the normal HRMCLi workspace.
     */
    show_startup_screen();

    /*
     * Initialize global TUI state.
     */
    state.current_screen =
        SCREEN_MAIN_MENU;

    state.quit_requested = 0;

    screen_main_menu_init(
        &state.main_menu,
        empty_rect
    );

    screen_nodes_init(
        &state.nodes
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

    primary.draw =
        draw_primary_pane;

    primary.handle_key =
        handle_primary_pane_key;

    /*
     * The optional secondary pane is currently
     * always the HRMCLi command interface.
     */
    secondary.draw =
        screen_cli_draw;

    secondary.handle_key =
        screen_cli_handle_key;

    /*
     * Determine initial workspace dimensions.
     */
    screen =
        ui_get_screen_rect();

    content =
        ui_rect_inset(
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
     * Main HRMCLi event loop.
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
             * Nodes has its own nested navigation.
             * Give it the first opportunity to
             * consume Escape.
             */
            if (
                state.current_screen ==
                SCREEN_NODES
            ) {
                if (
                    screen_nodes_handle_escape(
                        &state.nodes
                    )
                ) {
                    draw_screen(
                        &workspace
                    );

                    continue;
                }
            }

            /*
             * Otherwise Esc returns the primary
             * pane to the main menu.
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
         * Pass ordinary input to the currently
         * focused pane.
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
