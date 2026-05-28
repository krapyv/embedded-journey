# LED Blink Reusable Library

This project provides a highly reusable, hardware-agnostic driver for initializing and toggling LEDs on STM32 microcontrollers. By avoiding hardcoded register addresses, GPIO port, or pin numbers, the library allows for dynamic multi-LED configurations.
This implementation operates completely bare-metal without relying on the vendor HAL or external abstraction layer, utilizing direct register manipulation and standard C data types.

## 📂 Project Structure

```text
├── led.c       # Initialization and toggle functions implementation
└── led.h       # Struct layouts, configuration handles, and function prototypes   
```

## 🧠 Core Low-Level Concepts
### Memory Map Base Addresses

* **Dynamic Peripheral Mapping:** The absolute memory addresses for the clock enable register (`rcc_clk_reg`), GPIO mode register (`moder_reg`), and GPIO output data register (`odr_reg`) are passed at runtime via an instance of the configuration handle (`LED_HandleTypeDef`).
* **Pin and Clock Bit Passing:** The specific GPIO pin number and the bit position in RCC_AHB1ENR are defined dynamically inside the handle, allowing a single driver file to manage different pins across entirely separate GPIO ports (e.g., Port A, Port C).

### Register Configuration & Bitwise Operations
This driver dynamically configures and modifies the hardware registers using explicit bitwise masking and shifting relative to the values passed in the configuration structure:

1. Clock Activation (`RCC_AHB1ENR`):

The clock domain for the target GPIO port is enabled by setting the corresponding bit position via a read-modify-write bitwise OR operation:

```c
    *hled->rcc_clk_reg |= (1 << hled->rcc_bit);
```

2. GPIO Mode Configuration (`GPIOx_MODER`):

Each GPIO pin takes up a 2-bit field in the MODER register to define its direction. To set a pin safely as a **General Purpose Output (01)**, the driver first clears the existing 2-bit field using a bitwise AND mask, then shifts and ORs the `01` configuration state:


```c
    *hled->moder_reg &= ~((0x3 << (2 * hled->pin))); // Clear the existing 2 bits
    *hled->moder_reg |= (0x1 << (2 * hled->pin)); // Set mode to Output (01)
```

3. GPIO Output Control (`odr_reg`):

This driver exposes explicit functions to assert or de-assert the pin state cleanly: 

- Turning the LED on is handled by setting the corresponding bit position via a read-modify-write bitwise OR operation:

```c
    *hled->odr_reg |= (1 << hled->pin);
```

- Turning the LED off is handled by clearing the corresponding bit position via a read-modify-write bitwise AND operation performed with a bitwise inversion:

```c
    *hled->odr_reg &= ~(1 << hled->pin);
```

## 🚀 Compilation and Integration
### Compiling as an Isolated Object File

Since this library does not contain a `main()` function, it is compiled directly into a reusable object file (`.o`) using the GNU ARM Embedded Toolchain (`arm-none-eabi-gcc`).

Run the following command to compile the source code without invoking the linker:
```bash
arm-none-eabi-gcc -c -mcpu=cortex-m4 -mthumb -Wall -O2 led.c -o led.o
```
- `-c`: Compiled the source file into an object file (`led.o`) but skips the linking phase.
- `-mcpu=cortex-m4 -mthumb`: Targets the specific ARM Cortex-M4 architecture layout of the STM32.

### How to Integrate This Library into Your Project

To use this library in an application, include the header file in your main application file and link the compiled object files together.

```c
#include "led.h"

int main(void) {
    LED_HandleTypeDef led_pa5;
    led_pa5.moder_reg = (volatile uint32_t *)(0x40020000 + 0x00);
    led_pa5.odr_reg = (volatile uint32_t *)(0x40020000 + 0x14);
    led_pa5.rcc_clk_reg = (volatile uint32_t *)(0x40023800 + 0x30);
    led_pa5.pin = 5;
    led_pa5.rcc_bit = 0;

    led_init(&led_pa5);

    led_on(&led_pa5);

    while(1) {
        if (ms_passed >= 4000) {
            led_off(&led_pa5);
        }
    }
}
```

### Final Linking Command

When building your final application image, link your application object files alongside this library's object file:

```bash
arm-none-eabi-gcc main.o led.o -mcpu=cortex-m4 -mthumb -T linker_script.ld -o final_application.elf
```

## 🗺️  Next Steps & Learning Roadmap

To make this driver fully production-ready for automotive deployment, the next development steps for this library are:
1. **Hardware-level Debouncing Integration:** Pair this output driver with a basic input capture or GPIO reading module to manage physical button inputs safely without false edge triggering.
2. **Current-Limiting Hardare Diagnostics:** Document the exact hardware constraints, such as calculating necessary series resistance ($R = \frac{V_{OH} - V_F}{I_F}$) to prevent over-current degradation of the internal GPIO switching transistor.
3. **Bit-Banged Protocol Abstraction:** Use this base driver to build low-level, bit-banged software implementations for simple data transmission lines (e.g., synchronous clock generation or custom serial data streams).

## 🪪 Author & License
* **Author:** Dmytro Krapyvianskyi
* **License:** This project is open-source software licensed under the MIT License.
