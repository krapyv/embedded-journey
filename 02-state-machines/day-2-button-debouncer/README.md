# Button Debouncer

A finite state machine implementation of a button debouncer with 4 states (RELEASED, PRESSING, PRESSED, RELEASING) using non-blocking timers and configurable debounce delay. Built in C with no external libraries.

## Overview

A 4-state finite state machine (RELEASED -> PRESSING -> PRESSED -> RELEASING) that filters mechanical bounce from a button input using a configurable debounce delay. The system uses non-blocking timers to track state duration and debounce delays to prevent multiple unstable signals from being registered as valid button presses.

## State Diagram

RELEASED - (button pressed) -> PRESSING 
PRESSING - (stable for 20ms) -> PRESSED
PRESSED - (button released) -> RELEASING
RELEASING - (stable for 20ms) -> RELEASED

## Features

- **4-state button debouncing sequence**: RELEASED -> PRESSING -> PRESSED -> RELEASING
- **Configurable debounce delay**: Pass any delay (in milliseconds) to `button_debouncer_init()`
- **Non-blocking timers**: Counter-based timing (no `delay()` functions)
- **Bounce rejection**: Only stable signals (held for debounce duration) register as valid
- **Returns debounced state**: 0 for released, 1 for pressed
- **Test harness included**: 500ms simulation with programmable bounce patterns

## Requirements

- GCC or any other C compiler
- Standard C libraries

## Building

```bash
gcc button_debouncer.c test_button_debouncer.c -o test_button_debouncer
./test_button_debouncer
```

## Usage

```c
#include <stdint.h>
#include <stdbool.h>
#include "button_debouncer.h"

#define DEFAULT_DEBOUNCE_DELAY 20

uint32_t current_time_ms = 0;

void timer_interrupt(void)
{
    current_time_ms += 5;
}

bool get_bouncing_button_signal_simpler(uint32_t time_ms)
{
    if (time_ms >= 100 && time_ms < 300)
    {
        uint32_t t = time_ms - 100;

        if (t < 10)
            return true;
        if (t < 15)
            return false;
        if (t < 25)
            return true;
        if (t < 30)
            return false;

        return true;
    }

    return false;
}

int main(void) {
    ButtonDebouncer_t button;

    button_debouncer_init(&button, DEFAULT_DEBOUNCE_DELAY);

    while (current_time_ms < 500)
    {
        bool raw_input = get_bouncing_button_signal_simpler(current_time_ms);

        bool debounced = button_debouncer_update(&button, raw_input, 5);

        timer_interrupt();
    }

    return 0;
}
```

## Output

```text
--- Button Debouncer ---
Debounce delay = 20
   0ms: raw=0, debounce=0
   ...
  90ms: raw=0, debounce=0
  95ms: raw=0, debounce=0
 100ms: raw=1, debounce=0
 105ms: raw=0, debounce=0
 110ms: raw=1, debounce=0
 115ms: raw=0, debounce=0
 120ms: raw=1, debounce=0
 125ms: raw=1, debounce=0
 130ms: raw=1, debounce=0
 135ms: raw=1, debounce=0
 140ms: raw=1, debounce=1
 145ms: raw=1, debounce=1
 150ms: raw=1, debounce=1
 155ms: raw=1, debounce=1
 160ms: raw=1, debounce=1
 165ms: raw=1, debounce=1
 170ms: raw=1, debounce=1
 175ms: raw=1, debounce=1
 180ms: raw=1, debounce=1
 185ms: raw=1, debounce=1
 190ms: raw=1, debounce=1
 195ms: raw=1, debounce=1
 200ms: raw=0, debounce=0
 205ms: raw=0, debounce=0
 210ms: raw=0, debounce=0
 215ms: raw=0, debounce=0
 ...
 305ms: raw=1, debounce=0
 310ms: raw=0, debounce=0
 315ms: raw=1, debounce=0
 320ms: raw=0, debounce=0
 ...
 495ms: raw=0, debounce=0

```

## Project Structure

```text
├── button_debouncer.h      # API for the debouncer logic
├── button_debouncer.c      # Switch-case state machine implementation 
├── test_button_debouncer.c # Testing application
└── README.md               # This .md file
```

## API Reference

## Button Debouncer Operations

| Function | Description |
| ---------| ----------- |
| button_debouncer_init() | Initialize the button debouncer state. |
| button_debouncer_update() | Update state machine timing and process bounce rejection. |

## State Machine States

| State | Duration | Output | Description |
| ----- | -------- | ---------- | --------- |
| RELEASED | N/A | 0 | Button not pressed. |
| PRESSING | Up to debounce delay | 0 | Debouncing press (output remains 0 until stable). |
| PRESSED | N/A | 1 | Button confirmed pressed. | 
| RELEASING | Up to debounce delay | 0 | Debouncing release (output already 0). |

## Author

Dmytro Krapyvianskyi

## License

MIT
