#include <stddef.h>
#include <string.h>

#include "hrmcli/menu.h"
#include "hrmcli/terminal.h"
#include "hrmcli/ui.h"


void ui_menu_init(
    UiMenu *menu,
    UiRect rect,
    const char **items,
    int item_count
)
{
    if (menu == NULL) {
        return;
    }

    menu->rect = rect;
    menu->items = items;
    menu->item_count = item_count;
    menu->selected = 0;
}


void ui_menu_set_rect(
    UiMenu *menu,
    UiRect rect
)
{
    if (menu == NULL) {
        return;
    }

    menu->rect = rect;
}


void ui_menu_draw(
    UiMenu *menu
)
{
    int row;

    if (
        menu == NULL ||
        menu->items == NULL ||
        menu->item_count <= 0
    ) {
        return;
    }

    for (int i = 0; i < menu->item_count; i++) {
        row = menu->rect.row + i;

        /*
         * Don't draw past the menu's assigned area.
         */
        if (i >= menu->rect.height) {
            break;
        }

        if (i == menu->selected) {
            ui_set_reverse(1);
        }

        ui_draw_text(
            row,
            menu->rect.col,
            menu->items[i]
        );

        /*
         * Fill the rest of the menu width so the
         * highlighted selection becomes a full bar.
         */
        int length =
            (int)strlen(menu->items[i]);

        for (
            int col = length;
            col < menu->rect.width;
            col++
        ) {
            ui_draw_text(
                row,
                menu->rect.col + col,
                " "
            );
        }

        if (i == menu->selected) {
            ui_set_reverse(0);
        }
    }
}


int ui_menu_handle_key(
    UiMenu *menu,
    int key
)
{
    if (
        menu == NULL ||
        menu->item_count <= 0
    ) {
        return UI_MENU_NO_SELECTION;
    }

    switch (key) {
    case TERMINAL_KEY_UP:
        menu->selected--;

        if (menu->selected < 0) {
            menu->selected =
                menu->item_count - 1;
        }

        break;

    case TERMINAL_KEY_DOWN:
        menu->selected++;

        if (
            menu->selected >=
            menu->item_count
        ) {
            menu->selected = 0;
        }

        break;

    case TERMINAL_KEY_HOME:
        menu->selected = 0;
        break;

    case TERMINAL_KEY_END:
        menu->selected =
            menu->item_count - 1;
        break;

    case TERMINAL_KEY_ENTER:
        return menu->selected;

    default:
        break;
    }

    return UI_MENU_NO_SELECTION;
}
