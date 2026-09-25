#ifndef HRMCLI_SCREEN_MAIN_MENU_H
#define HRMCLI_SCREEN_MAIN_MENU_H

#include "hrmcli/menu.h"
#include "hrmcli/pane.h"
#include "hrmcli/ui.h"

typedef enum {
    MAIN_MENU_DASHBOARD = 0,
    MAIN_MENU_NODES,
    MAIN_MENU_CONFIGURATION,
    MAIN_MENU_LOGS,
    MAIN_MENU_CLI,
    MAIN_MENU_SYSTEM,
    MAIN_MENU_QUIT,
    MAIN_MENU_COUNT
} MainMenuItem;

void screen_main_menu_init(
    UiMenu *menu,
    UiRect rect
);

void screen_main_menu_draw(
    UiPane *pane,
    UiMenu *menu
);

int screen_main_menu_handle_key(
    UiMenu *menu,
    int key
);

#endif
