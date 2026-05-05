#include "menu_system.h"
#include <stdio.h>

// to avoid using the magical numbers, define variables
#define MENU_DIAGNOSTICS_ITEMS 1
#define MENU_ABOUT_ITEMS 1
#define MENU_MAIN_ITEMS 3
#define MENU_SETTINGS_ITEMS 3
#define MENU_MIN_ITEMS 0

#define VOLUME_MIN 0
#define VOLUME_MAX 100

#define BRIGHTNESS_MIN 0
#define BRIGHTNESS_MAX 100

#ifdef TEST_MODE
#define ALWAYS_REDRAW 1
#else
#define ALWAYS_REDRAW 0
#endif

void menu_system_init(MenuSystem_t *menu, uint32_t delay_ms, uint32_t interval_ms)
{
    menu->current_state = MENU_MAIN;
    menu->selected_item = 0;
    menu->volume = 50;
    menu->brightness = 75;
    menu->screen_dirty = false;
    menu->repeat_delay = delay_ms;
    menu->repeat_interval = interval_ms;
    menu->repeat_timer_up = 0;
    menu->repeat_timer_down = 0;
    menu->repeat_active_up = false;
    menu->repeat_active_down = false;
    menu->last_up_state = false;
    menu->last_down_state = false;
}

bool handle_key_repeat(uint32_t *timer, bool button_pressed, uint32_t interval_ms, uint32_t delay_ms, uint32_t repeat_ms, bool *repeat_active, bool *last_state)
{
    static bool last_pressed = false;
    bool edge = (button_pressed && !*last_state);
    *last_state = button_pressed;

    if (button_pressed)
    {
        if (edge)
        {
            // on the first press, we should act immediately
            *timer = delay_ms;
            *repeat_active = 1;
            return true;
        }
        else if (*repeat_active)
        {
            if (*timer > interval_ms)
            {
                *timer -= interval_ms;
                return false;
            }
            else
            {
                // Repeat interval expired
                *timer = repeat_ms;
                return true;
            }
        }
    }
    else
    {
        *repeat_active = 0;
        *timer = 0;
    }

    return false;
}

// static represents a private helper used only inside the current .c file.
// No other file should call it directly
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

    menu->screen_dirty = true; // we need to render the new screen
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
    menu->screen_dirty = true;
}

// depending on a value in the current_state, the function should draw the entire current screen, clearing the previous one
void menu_display(MenuSystem_t *menu)
{
#ifdef TEST_MODE
    printf("\033[2J\033[H");
#else

    if (!(menu->screen_dirty))
    {
        return;
    }

    // using ASCII control sequence to clear the entire terminal window and move the cursor to the "home" position
    printf("\033[2J\033[H");
#endif

    switch (menu->current_state)
    {
    case MENU_MAIN:
        printf("=== MAIN MENU ===\n");
        printf("%s Settings\n", menu->selected_item == 0 ? ">" : " ");
        printf("%s Diagnostics\n", menu->selected_item == 1 ? ">" : " ");
        printf("%s About\n", menu->selected_item == 2 ? ">" : " ");
        break;

    case MENU_SETTINGS:
        printf("=== SETTINGS ===\n");
        printf("%s Volume: %d\n", menu->selected_item == 0 ? ">" : " ", menu->volume);
        printf("%s Brightness: %d\n", menu->selected_item == 1 ? ">" : " ", menu->brightness);
        printf("%s Back\n", menu->selected_item == 2 ? ">" : " ");
        break;

    case MENU_EDIT_VOLUME:
        printf("=== EDIT VOLUME ===\n");
        printf("Value: %d\n", menu->volume);
        printf("\nUse Up/Down to change\n");
        printf("Press Enter to save and leave\n");
        break;

    case MENU_EDIT_BRIGHTNESS:
        printf("=== EDIT BRIGHTNESS ===\n");
        printf("Brightness: %d\n", menu->brightness);
        printf("\nUse Up/Down to change\n");
        printf("Press Enter to save and leave\n");
        break;

    case MENU_DIAGNOSTICS:
        printf("=== DIAGNOSTICS ===\n");
        printf("Volume: %d\n", menu->volume);
        printf("Brightness: %d\n", menu->brightness);
        printf("\n[Back]\n");
        break;

    case MENU_ABOUT:
        printf("=== ABOUT ===\n");
        printf("Menu System v1.0\n");
        printf("Embedded FSM Demo\n");
        printf("Author: Dmytro Krapyvianskyi\n");
        printf("\n[Back]\n");
        break;
    }

#ifndef TEST_MODE
    menu->screen_dirty = false;
#endif
}

void menu_system_update(MenuSystem_t *menu, bool enter, bool up, bool down, uint32_t interval_ms)
{
    if (up && down)
    {
        return;
    }

    // we are calling handle_key_repeat for each button we want to repeat
    bool up_action = handle_key_repeat(&menu->repeat_timer_up, up, interval_ms, menu->repeat_delay, menu->repeat_interval, &menu->repeat_active_up, &menu->last_up_state);
    bool down_action = handle_key_repeat(&menu->repeat_timer_down, down, interval_ms, menu->repeat_delay, menu->repeat_interval, &menu->repeat_active_down, &menu->last_down_state);

    switch (menu->current_state)
    {
    case MENU_MAIN:
        if (enter)
        {
            menu_main_handle_enter(menu);
        }
        else if (up_action)
        {
            if (menu->selected_item > MENU_MIN_ITEMS)
            {
                menu->selected_item--;
                menu->screen_dirty = true; // logically, we need to redraw the screen only if the selected item has changed. otherwise the content stays the same so no redrawing is needed
            }
        }
        else if (down_action)
        {
            if (menu->selected_item < MENU_MAIN_ITEMS - 1)
            {
                menu->selected_item++;
                menu->screen_dirty = true;
            }
        }
        break;

    case MENU_SETTINGS:
        if (enter)
        {
            menu_settings_handle_enter(menu);
        }
        else if (up_action)
        {
            if (menu->selected_item > MENU_MIN_ITEMS)
            {
                menu->selected_item--;
                menu->screen_dirty = true;
            }
        }
        else if (down_action)
        {
            if (menu->selected_item < MENU_SETTINGS_ITEMS - 1)
            {
                menu->selected_item++;
                menu->screen_dirty = true;
            }
        }
        break;

    case MENU_DIAGNOSTICS:
        if (enter)
        {
            if (menu->selected_item == (MENU_DIAGNOSTICS_ITEMS - 1))
            {
                menu->current_state = MENU_MAIN;
                menu->selected_item = 1;
                menu->screen_dirty = true;
            }
        }
        break;

    case MENU_ABOUT:
        if (enter)
        {
            if (menu->selected_item == (MENU_ABOUT_ITEMS - 1))
            {
                menu->current_state = MENU_MAIN;
                menu->selected_item = 2;
                menu->screen_dirty = true;
            }
        }
        break;

    case MENU_EDIT_VOLUME:
        // we are using Up/Down keys for increments/decrement
        if (up_action)
        {
            if (menu->volume < VOLUME_MAX)
            {
                menu->volume++;
                menu->screen_dirty = true;
            }
        }
        else if (down_action)
        {
            if (menu->volume > VOLUME_MIN)
            {
                menu->volume--;
                menu->screen_dirty = true;
            }
        }
        else if (enter)
        { // back
            menu->current_state = MENU_SETTINGS;
            menu->selected_item = 0;
            menu->screen_dirty = true;
        }
        break;

    case MENU_EDIT_BRIGHTNESS:
        // we are using Up/Down keys for increments/decrement
        if (up_action)
        {
            if (menu->brightness < BRIGHTNESS_MAX)
            {
                menu->brightness++;
                menu->screen_dirty = true;
            }
        }
        else if (down_action)
        {
            if (menu->brightness > BRIGHTNESS_MIN)
            {
                menu->brightness--;
                menu->screen_dirty = true;
            }
        }
        else if (enter)
        { // back
            menu->current_state = MENU_SETTINGS;
            menu->selected_item = 1;
            menu->screen_dirty = true;
        }
        break;
    }

    // to always clear the output
    if (ALWAYS_REDRAW)
    {
        menu->screen_dirty = true;
    }
    menu_display(menu);
}