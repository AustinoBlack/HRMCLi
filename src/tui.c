#include <stddef.h>

#include "hrmcli/menu.h"
#include "hrmcli/pane.h"
#include "hrmcli/terminal.h"
#include "hrmcli/tui.h"
#include "hrmcli/ui.h"
#include "hrmcli/workspace.h"

static const char *main_menu_items[] = {
    "Dashboard",
    "Nodes",
    "Configuration",
    "Logs",
    "HRMCLi CLI",
    "System",
    "Quit"
};

static void draw_primary_pane(UiPane *pane)
{
    UiRect content;
    UiMenu *menu;

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

    menu = pane->userdata;

    if (menu == NULL) {
        return;
    }

    /*
     * Give the menu the current pane dimensions.
     * This keeps it responsive when the terminal resizes.
     */
    ui_menu_set_rect(
        menu,
        content
    );

    ui_menu_draw(menu);
}

static void draw_secondary_pane(UiPane *pane)
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

static void handle_primary_pane_key(
    UiPane *pane,
    int key
)
{
    UiMenu *menu;
    int selected;

    if (pane == NULL) {
        return;
    }

    menu = pane->userdata;

    if (menu == NULL) {
        return;
    }

    selected = ui_menu_handle_key(
        menu,
        key
    );

    /*
     * We are only testing selection for now.
     * Actual screen navigation comes next.
     */
    if (selected != UI_MENU_NO_SELECTION) {
        /*
         * Temporary behavior.
         */
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

    UiMenu main_menu;

    int key;
    int running = 1;

    /*
     * Initialize the terminal backend.
     */
    if (terminal_init() != 0) {
        return 1;
    }

    /*
     * Create the two panes.
     *
     * They begin with empty rectangles because
     * UiWorkspace will assign their real geometry.
     */
    ui_pane_init(
        &primary,
        empty_rect,
        "NODE INFO"
    );

    ui_pane_init(
        &secondary,
        empty_rect,
        "HRMCLI CLI"
    );

    ui_menu_init(
        &main_menu,
        empty_rect,
        main_menu_items,
        7
    );

    /*
     * Assign the content-drawing callbacks.
     */
    primary.userdata = &main_menu;
    primary.draw = draw_primary_pane;
    primary.handle_key = handle_primary_pane_key;

    secondary.draw = draw_secondary_pane;

    /*
     * Determine the initial area available
     * to the workspace.
     */
    screen = ui_get_screen_rect();

    content = ui_rect_inset(
        screen,
        1
    );

    /*
     * Initialize the workspace.
     *
     * The primary pane is visible by default.
     * The secondary pane starts hidden.
     */
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
     * Main TUI event loop.
     */
    while (running) {
        key = terminal_read_key();

        if (key < 0) {
            break;
        }

        switch (key) {
        /*
         * Recalculate the workspace whenever
         * the terminal size changes.
         */
        case TERMINAL_KEY_RESIZE:
            screen = ui_get_screen_rect();

            content = ui_rect_inset(
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

        /*
         * Tab switches focus between panes.
         *
         * If split view is not active,
         * UiWorkspace simply ignores this.
         */
        case TERMINAL_KEY_TAB:
            ui_workspace_toggle_focus(
                &workspace
            );

            draw_screen(
                &workspace
            );

            continue;

        /*
         * Normal termination paths.
         */
        case TERMINAL_KEY_TERMINATE:
        case TERMINAL_KEY_CTRL_C:
            running = 0;
            continue;

        default:
            break;
        }

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
         * Quit HRMCLi.
         */
        if (
            key == 'q' ||
            key == 'Q'
        ) {
            running = 0;
            continue;
        }

        /*
         * All other input is passed to the
         * currently focused pane.
         */
        ui_workspace_handle_key(
            &workspace,
            key
        );

        draw_screen(
            &workspace
        );
    }

    /*
     * Restore the user's terminal before exiting.
     */
    terminal_clear();
    terminal_move_cursor(1, 1);
    terminal_shutdown();

    return 0;
}
