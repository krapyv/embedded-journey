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
    uint32_t repeat_timer;
} MenuSystem_t;

#endif