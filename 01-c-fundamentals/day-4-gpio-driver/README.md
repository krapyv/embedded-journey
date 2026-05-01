# Bitwise GPIO Driver

A simulated GPIO driver demonstration bitwise operations and hardware abstraction in C.

## Overview

This project implements a complete GPIO (General-Purpose Input/Output) driver using bitwise operations.
It simulates a 32-bit GPIO peripheral and provides a clean API for pin and port manipulation. 
The code is written in standard C. No external libraries required.

## Features

- **Single pin operations**: Set, clear, toggle, and read individual pins
- **Multiple pin operations**: Write and read ranges of pins (e.g., bits 7-4)
- **Whole port operations**: Write and read entire 32-bit port values
- **Reusable bitwise library**: Can be used on real STM32 registers, independent of hardware
- **No external dependencies**: No library required

## Requirements

- GCC or any other C compiler
- Standard C libraries

## Building

```bash
gcc test_gpio.c bitwise.c simulated_gpio.c -o test_gpio
./test_gpio
```

## Usage

```c
#include "simulated_gpio.h"

uint32_t port = 0;

// Set pin 5 HIGH (1)
gpio_set_pin(&port, 5);

// Set pin 5 LOW (0)
gpio_clear_pin(&port, 5);

// Write binary 1010 to pins 7-4
gpio_write_pins(&port, 7, 4, 0b1010);

// Read bits 7-4
uint32_t value = gpio_read_pins(port, 7, 4); // value = 10 (0x0A)
```

## Output

```text
Initial                            : 0x00000000
gpio_set_pin(&port, 5)             : 0x00000020
gpio_clear_pin(&port, 5)           : 0x00000000
gpio_toggle_pin(&port, 3)          : 0x00000008
gpio_read_pin(port, 3)             : 1
gpio_write_pins(&port, 7, 4, 0b1010): 0x000000A8
gpio_read_pins(port, 7, 4)         : 0x0000000A
gpio_write_port(&port, 0x12345678) : 0x12345678
gpio_read_port(port)               : 0x12345678
```

## Project Structure

```text
├── bitwise.h           # API for bit manipulation
├── bitwise.c           # Set, clear, toggle, extract, replace
├── simulated_gpio.h    # API for GPIO abstraction
├── simulated_gpio.c    # GPIO functions using bitwise
├── test_gpio.c          # Testing application
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

## Author

Dmytro Krapyvianskyi

## License

MIT
