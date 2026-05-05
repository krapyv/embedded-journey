#ifndef MENU_SYSTEM_H
#define MENU_SYSTEM_H

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    MENU_MAIN,
    MENU_SETTINGS,
    MENU_DIAGNOSTICS,
    MENU_ABOUT,
    MENU_EDIT_VOLUME,
    MENU_EDIT_BRIGHTNESS
} MenuSystemState_t;

typedef struct
{
    MenuSystemState_t current_state;
    uint8_t selected_item;
    uint8_t volume;
    uint8_t brightness;
    bool screen_dirty;

    uint32_t repeat_timer_up; // current countdown
    uint32_t repeat_timer_down;
    bool repeat_active_up;    // true if we are currently in repeat mode
    bool repeat_active_down;  // true if we are currently in repeat mode
    uint32_t repeat_delay;    // the delay before finst repeat
    uint32_t repeat_interval; // the interval between repeats

    bool last_up_state;
    bool last_down_state;
} MenuSystem_t;

void menu_system_init(MenuSystem_t *menu, uint32_t delay_ms, uint32_t interval_ms);
void menu_display(MenuSystem_t *menu);
void menu_system_update(MenuSystem_t *menu, bool enter, bool up, bool down, uint32_t interval_ms);

#endif