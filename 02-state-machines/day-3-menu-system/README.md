# Menu System

A finite state machine implementation of a menu system that navigates through screens and allows value adjustment. Built in C with no external libraries.

## Overview

A 6-state finite state machine (MENU_MAIN, MENU_SETTINGS, MENU_DIAGNOSTICS, MENU_ABOUT, MENU_EDIT_VOLUME, MENU_EDIT_BRIGHTNESS) that provides clean, maintainable menu logic to navigate through screens and adjust values. The system uses non-blocking key repeat handling using configurable timers and renders menu screens to the terminal.

## State Diagram

Initial State: MENU_MAIN (selected_item = 0)

MENU_MAIN - [ENTER on Settings] -> MENU_SETTINGS
MENU_MAIN - [ENTER on Diagnostics] -> MENU_DIAGNOSTICS
MENU_MAIN - [ENTER on About] -> MENU_ABOUT

MENU_SETTINGS - [ENTER on Volume] -> MENU_EDIT_VOLUME
MENU_SETTINGS - [ENTER on Brightness] -> MENU_EDIT_BRIGHTNESS
MENU_SETTINGS - [ENTER on Back] -> MENU_MAIN

MENU_DIAGNOSTICS -[ENTER]-> MENU_MAIN
MENU_ABOUT -[ENTER]-> MENU_MAIN

MENU_EDIT_VOLUME -[ENTER]-> MENU_SETTINGS (selected_item = 0)
MENU_EDIT_BRIGHTNESS -[ENTER]-> MENU_SETTINGS (selected_item = 1)

## Features

- **6-state finite state machine**: MAIN -> SETTINGS -> EDIT VOLUME -> EDIT BRIGHTNESS -> DIAGNOSTICS -> ABOUT
- **Non-blocking configurable key repeat**: Configurable delay and repeat intervals for UP/DOWN buttons
- **Screen dirty flag optimization**: Redraws when content changes
- **Value adjustment**: Volume (0-100) and brightness (0-100) adjustment with bounds checking
- **Portable**: Console output for testing, replaceable with real display drivers
- **Modularity**: Clean separation between menu logic, display, and input handling

## Key Repeat Configuration

| Parameter | Default | Description |
| --------- | ------- | ----------- |
| delay_ms | 500ms | Time before repeat starts (first repeat after initial press) |
| interval_ms | 100ms | Time between subsequent repeats after delay |
| interval_ms (call) | 10ms | How often `menu_system_update()` is called |

**Timing example**:
- 0ms: Button pressed -> immediate action
- 0-500ms: No repeats (delay period)
- 500ms: First repeat action
- 600ms: Second repeat action
- 700ms: Third repeat action (every 10ms)

## Requirements

- GCC or any other C compiler
- Standard C libraries

## Building

```bash
gcc -DTEST_MODE menu_system.c test_menu_system.c -o test_menu_system
./test_menu_system
```
where -DTEST_MODE defines the macro TEST_MODE with a default value of 1 (true).

## Production Building (No test mode)

```bash
gcc menu_system.c test_menu_system.c -o test_menu_system
./test_menu_system
```

## Usage

```c
#include "menu_system.h"
MenuSystem_t menu;

menu_system_init(&menu, 0, 0);

int actions[][4] = {
    {0, 0, 1, 15},
    {0, 0, 1, 15},
    {0, 1, 0, 15}, 
    {0, 1, 0, 15},
    {1, 0, 0, 15},
    {1, 0, 0, 15},
    {0, 1, 0, 15},
    {0, 1, 0, 15},
    {1, 0, 0, 15},
    {0, 0, 1, 15},
    {0, 0, 1, 15},
    {1, 0, 0, 15}
};

for (int row = 0; row < 12; row++)
{
    menu_system_update(&menu, actions[row][0], actions[row][1], actions[row][2], actions[row][3]);
}
```

## Output

```text
=== MAIN MENU ===
  Settings
> Diagnostics
  About


=== MAIN MENU ===
  Settings
  Diagnostics
> About


=== MAIN MENU ===
  Settings
> Diagnostics
  About


=== MAIN MENU ===
> Settings
  Diagnostics
  About


=== SETTINGS ===
> Volume: 50
  Brightness: 75
  Back


=== EDIT VOLUME ===
Value: 50

Use Up/Down to change
Press Enter to save and leave


=== EDIT VOLUME ===
Value: 51

Use Up/Down to change
Press Enter to save and leave


=== EDIT VOLUME ===
Value: 52

Use Up/Down to change
Press Enter to save and leave


=== SETTINGS ===
> Volume: 52
  Brightness: 75
  Back


=== SETTINGS ===
  Volume: 52
> Brightness: 75
  Back


=== SETTINGS ===
  Volume: 52
  Brightness: 75
> Back


=== MAIN MENU ===
> Settings
  Diagnostics
  About
```

## Project Structure

```text
├── menu_system.h      # API for the menu logic
├── menu_system.c      # Switch-case state machine implementation 
├── test_menu_system.c # Testing application
└── README.md          # This .md file
```

## API Reference

## Menu System Operations

| Function | Description |
| ---------| ----------- |
| menu_system_init() | Initialize the menu system state. |
| menu_system_update() | Update state machine and process key handling. |

## Menu System Helper Functions

| Function | Description |
| -------- | ----------- |
| handle_key_repeat | Handles key repeat logic |
| menu_main_handle_enter | Processes Enter press in MAIN menu |
| menu_settings_handle_enter | Processes Enter press in SETTINGS menu |
| menu_display | Renders current screen to display, clearing the previous content |

## State Machine States

| State | Description | Items/Actions | Parent |
| ----- | ----------- | ------------- | ------ |
| MENU_MAIN | Main menu screen | Settings, Diagnostics, About | None (root) |
| MENU_SETTINGS | Settings submenu - allows access to configurable parameters | Volume, Brightness, Back | MENU_MAIN |
| MENU_DIAGNOSTICS | Displays current system status | Volume (readonly), Brightness (readonly) | MENU_MAIN |
| MENU_ABOUT | Displays system information | Version, name, author | MENU_MAIN |
| MENU_EDIT_VOLUME | Allows modifying volume level | Value 0-100, Up/Down controls, Enter to save | MENU_SETTINGS |
| MENU_EDIT_BRIGHTNESS | Allows modifying brightness level | Brightness 0-100, Up/Down controls, Enter to save | MENU_SETTINGS |

## Author

Dmytro Krapyvianskyi

## License

MIT
