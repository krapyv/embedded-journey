# Traffic Light Controller

A finite state machine implementation of a traffic light controller with pedestrian request handling, demonstration non-blocking timer and proper state transitions. Built in C on top of a reusable GPIO driver.

## Overview

A 4-state finite state machine (RED -> RED+YELLOW -> GREEN -> YELLOW) that controls a traffic light with pedestrian request support. The system uses non-blocking timers to track state duration and responds to pedestrian button presses by shortening the GREEN phase (minimum 4 seconds) before cycling to RED.
Written in C with a hardware-abstraction GPIO layer for portability.

## State Diagram

RED (5s) -> RED+YELLOW (1s) -> GREEN (7s) -> YELLOW (2s) -> RED

## Features

- **4-state traffic light sequence**: RED -> RED+YELLOW -> GREEN -> YELLOW -> RED
- **Pedestrian request handling**: Button press during GREEN sets pending flag
- **Mininum GREEN time**: Configurable 4-seconds minimum before pedestrian crossing
- **Non-blocking timers**: Counter-based timing (no `delay()` functions)
- **Configurable constants**: All durations defined with `#define` (no magic numbers)
- **Reusable GPIO driver**: Hardware abstraction via `simulated_gpio.h`
- **Test harness included**: 60-second simulation with pedestrian press at 11 seconds

## Requirements

- GCC or any other C compiler
- Standard C libraries

## Building

```bash
gcc bitwise.c simulated_gpio.c traffic_light.c test_traffic_light.c -o test_traffic_light
./test_traffic_light
```

## Usage

```c
#include <stdio.h>
#include "simulated_gpio.h"
#include "traffic_light.h"

uint32_t port = 0;

TrafficLight_t light;
TrafficLightPins_t pins = {
    .red = 0,
    .yellow = 1,
    .green = 2
};

traffic_light_init(&light, &port, pins);

for (int i = 0; i < 6000; i++) {
    bool pedestrian_signal = (i >= 1100 && i < 1101);
    traffic_light_update(&light, pedestrian_signal, 10);
}
```

## Output

```text
=== Traffic Lights Test ===
Time: 0.0s, State: 0
LEDs: R=1 Y=0 G=0 Time: 1.0s, State: 0
LEDs: R=1 Y=0 G=0 Time: 2.0s, State: 0
LEDs: R=1 Y=0 G=0 Time: 3.0s, State: 0
LEDs: R=1 Y=0 G=0 Time: 4.0s, State: 0
LEDs: R=1 Y=0 G=0 Time: 5.0s, State: 1
LEDs: R=1 Y=1 G=0 Time: 6.0s, State: 2
LEDs: R=0 Y=0 G=1 Time: 7.0s, State: 2
LEDs: R=0 Y=0 G=1 Time: 8.0s, State: 2
LEDs: R=0 Y=0 G=1 Time: 9.0s, State: 2
LEDs: R=0 Y=0 G=1 Time: 10.0s, State: 2
LEDs: R=0 Y=0 G=1 Time: 11.0s, State: 3
LEDs: R=0 Y=1 G=0 Time: 12.0s, State: 3
LEDs: R=0 Y=1 G=0 Time: 13.0s, State: 0
LEDs: R=1 Y=0 G=0 Time: 14.0s, State: 0
```

## Project Structure

```text
├── bitwise.h           # API for bit manipulation
├── bitwise.c           # Set, clear, toggle, extract, replace
├── simulated_gpio.h    # API for GPIO abstraction
├── simulated_gpio.c    # GPIO functions using bitwise
├── traffic_light.h     # API for traffic light controller
├── traffic_light.c     # Switch-case state machine implementation 
├── test_traffic_light.c          # Testing application
└── README.md           # This .md file
```

## API Reference

### Bitwise Operations


| Function | Description |
| ------------- | ------------- |
| set_bit(ptr, n)  | Set bit n to 1 |
| clear_bit(ptr, n)  | Set bit n to 0 |
| toggle_bit(ptr, n) | Flip bit n |
| is_bit_set(value, n) | Return 1 if bit n is 1, otherwise - 0 |
| extract_bits(value, high, low) | Return bits [high:low] bottom-aligned |
| replace_bits(ptr, high, low, new) | Replace bits [high:low] width new |

### GPIO Operations

| Function  | Description |
| ------------- | ------------- |
| gpio_set_pin(ptr, pin)  | Set pin high  |
| gpio_clear_pin(ptr, pin) | Set pin low  |
| gpio_toggle_pin(ptr, pin)| Toggle pin |
| gpio_read_pin(value, pin) | Read pin state |
| gpio_write_pins(ptr, high, low, value) | Write to pin range |
| gpio_read_pins(value, high, low) | Read pin range |
| gpio_write_port(ptr, value) | Write entire port |
| gpio_read_port(val) | Read entire port |

## Traffic light Operations

| Function | Description |
| ---------| ----------- |
| traffic_light_init() | Initialize the traffic light state |
| traffic_light_update() | Update state machine timing and process pedestrian request |
| traffic_light_green_to_yellow() | Helper function that transitions from GREEN to YELLOW |

## State Machine States

| State | Duration | LED Output |
| ----- | -------- | ---------- |
| TRAFFIC_RED | 5000 ms | RED on, YELLOW off, GREEN off |
| TRAFFIC_RED_YELLOW | 1000ms | RED on, YELLOW on, GREEN off |
| TRAFFIC_GREEN | 7000ms (4000ms minimum with pedestrian) | RED off, YELLOW off, GREEN on |
| TRAFFIC_YELLOW | 2000ms | RED off, YELLOW on, GREEN off |

## Author

Dmytro Krapyvianskyi

## License

MIT
