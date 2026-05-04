#include "menu_system.h"

void menu_system_init(MenuSystem_t *menu)
{
    menu->current_state = MENU_MAIN;
    menu->selected_item = 0;
    menu->volume = 50;
    menu->brightness = 75;
    menu->screen_dirty = 0;
    menu->repeat_timer = 0;
}

static void menu_main_handle_enter(MenuSystem_t *menu)
{
    switch (menu->selected_item)
    {
    case 0:
        menu->current_state = MENU_SETTINGS;
        break;

    case 1:
        menu->current_state = MENU_DIAGNOSTICS;
        break;

    case 2:
        menu->current_state = MENU_ABOUT;
        break;
    }

    menu->screen_dirty = 1; // we need to render the new screen
    menu->selected_item = 0;
}

static void menu_settings_handle_enter(MenuSystem_t *menu)
{
    switch (menu->selected_item)
    {
    case 0:
        menu->current_state = MENU_EDIT_VOLUME;
        break;

    case 1:
        menu->current_state = MENU_EDIT_BRIGHTNESS;
        break;

    // back
    case 2:
        menu->current_state = MENU_MAIN;
        break;
    }

    menu->selected_item = 0;
    menu->screen_dirty = 1;
}

void menu_system_update(MenuSystem_t *menu, bool enter, bool up, bool down)
{
    if (enter && up && down || enter && up || up && down || enter && down)
    {
        return;
    }

    switch (menu->current_state)
    {
    case MENU_MAIN:
        if (enter)
        {
            menu_main_handle_enter(menu);
        }
        else if (up)
        {
            if (menu->selected_item > 0)
            {
                menu->selected_item--;
                menu->screen_dirty = 1; // logically, we need to redraw the screen only if the selected item has changed. otherwise the content stays the same so no redrawing is needed
            }
        }
        else if (down)
        {
            if (menu->selected_item < 2)
            {
                menu->selected_item++;
                menu->screen_dirty = 1;
            }
        }
        break;

    case MENU_SETTINGS:
        if (enter)
        {
            menu_settings_handle_enter(menu);
        }
        else if (up)
        {
            if (menu->selected_item > 0)
            {
                menu->selected_item--;
                menu->screen_dirty = 1;
            }
        }
        else if (down)
        {
            if (menu->selected_item < 2)
            {
                menu->selected_item++;
                menu->screen_dirty = 1;
            }
        }
        break;

    case MENU_DIAGNOSTICS:
        break;

    case MENU_ABOUT:
        break;

    case MENU_EDIT_VOLUME:
        // we are using Up/Down keys for increments/decrement
        if (up)
        {
            if (menu->volume < 100)
            {
                menu->volume++;
                menu->screen_dirty = 1;
            }
        }
        else if (down)
        {
            if (menu->volume > 0)
            {
                menu->volume--;
                menu->screen_dirty = 1;
            }
        }
        break;

    case MENU_EDIT_BRIGHTNESS:
        // we are using Up/Down keys for increments/decrement
        if (up)
        {
            if (menu->brightness < 100)
            {
                menu->brightness++;
                menu->screen_dirty = 1;
            }
        }
        else if (down)
        {
            if (menu->brightness > 0)
            {
                menu->brightness--;
                menu->screen_dirty = 1;
            }
        }
        break;
    }
}