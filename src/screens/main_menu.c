#include <stddef.h>
#include "hrmcli/screens/main_menu.h"

static const char *main_menu_items[] = {
    "Dashboard",
    "Nodes",
    "Configuration",
    "Logs",
    "HRMCLi CLI",
    "System",
    "Quit"
};

void screen_main_menu_init(
    UiMenu *menu,
    UiRect rect
)
{
    ui_menu_init(
        menu,
        rect,
        main_menu_items,
        MAIN_MENU_COUNT
    );
}

void screen_main_menu_draw(
    UiPane *pane,
    UiMenu *menu
)
{
    UiRect content;

    if (
        pane == NULL ||
        menu == NULL
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

    ui_menu_set_rect(
        menu,
        content
    );

    ui_menu_draw(menu);
}

int screen_main_menu_handle_key(
    UiMenu *menu,
    int key
)
{
    if (menu == NULL) {
        return UI_MENU_NO_SELECTION;
    }

    return ui_menu_handle_key(
        menu,
        key
    );
}
