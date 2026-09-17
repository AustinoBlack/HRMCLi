#ifndef HRMCLI_MENU_H
#define HRMCLI_MENU_H

#include "hrmcli/ui.h"

#define UI_MENU_NO_SELECTION (-1)

typedef struct {
    UiRect rect;

    const char **items;
    int item_count;

    int selected;
} UiMenu;

void ui_menu_init(
    UiMenu *menu,
    UiRect rect,
    const char **items,
    int item_count
);

void ui_menu_set_rect(
    UiMenu *menu,
    UiRect rect
);

void ui_menu_draw(
    UiMenu *menu
);

/*
 * Returns:
 *
 * UI_MENU_NO_SELECTION
 *     No menu item was activated.
 *
 * 0 .. item_count - 1
 *     Enter was pressed on that item.
 */
int ui_menu_handle_key(
    UiMenu *menu,
    int key
);

#endif
