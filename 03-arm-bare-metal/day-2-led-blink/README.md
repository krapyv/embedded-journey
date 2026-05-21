# Button-Controlled LED Blink - STM32F411CEU6 Bare Metal

A lightweight, bare-metal C application that tracks interaction via a tactile push-button on pin PB0 to control an external LED connected to pin **PA5** on the STM32F411 (Blackpill) development board.
This repository implements a complete low-level driver stack without relying on standard vendor libraries or external HAL frameworks.
Button noise filtering is handled via a dedicated algorithmic software state-machine debouncer developed during the *02-state-machines* learning stage.

## 📋 Hardware Connections
The circuit was constructed on an 830-point breadboard using an **Active-Low configuration** with external pull-up resistor:
* **STM32F411 Pin PB0**: Connected to a row sharing one terminal of the **tactile push-button** and a **10KΩ Pull-Up Resistor**.
* **10KΩ resistor (Terminal 2)**: Tied directly to the **3V3 pin** of the Blackpill board.
* **Tactile Button (Ground Terminal)**: Tied directly to the common **GND rail** of the Blackpill
* **STM32F411 Pin PA5**: Connected to a row sharing one terminal of a **220Ω Current-Limiting Resistor**.
* **220Ω Resistor (Terminal 2)**: Connected to the **Anode (longer leg)** of the LED.
* **LED Cathode (shorter leg)**: Connected to a common ground rail tied directly to the **GND pin of the Blackpill**.

> **Active-Low Logic Event:** While the button remains unpressed, the 10KΩ resistor pulls the input pin up to 3.3V. Pressing the switch shorts the trace directly to ground, driving the pin to 0V. 

## 📂 Project Structure

```text
├── led_blink.c       # Main application logic and busy-wait delay loop
├── led_blink.h       # Macro layouts and register maps
├── button_debouncer.c      # Algorithmic state-machine debouncer logic implementation
├── button_debouncer.h      # Debouncer states, thresholds, and abstract APIs
├── stm32f411.ld      # Linker script defining Flash/SRAM memory segments
├── startup_stm32f411ceux.s  # Assembly startup file implementing vector table & Reset_Handler 
└── Makefile         #GNU Make automation configuration to compile and flash the system    
```

## 🧠 Core Low-Level Concepts
### Memory Map Base Addresses
* **`RCC_BASE`** (`0x40023800`): Base memory address for the Reset and Clock Control peripheral.
* **`GPIOA_BASE`** (`0x40020000`): Base memory address for General Purpose I/O Port A registers.
* **`GPIOB_BASE`** (`0x40020400`): Base memory address for General Purpose I/O Port B registers.


### Register Configuration & Bitwise Operations

* **`RCC_AHB1ENR`**(Offset: `0x30`)
 * **Purpose:** Enables peripheral clock gating for devices connected to the Advanced High-Performance Bus 1 (AHB1). Peripherals remain unclocked by default to minimize power consumption.
 * **Operation:** Bits 0 and 1 map directly to GPIOA and GPIOB accordingly. Setting these bits via bitwise OR masks activates the ports:
 ```c
    RCC_AHB1ENR |= (1 << 0);
    RCC_AHB1ENR |= (1 << 1);
 ```

* **`GPIOA_MODER`**(Offset: `0x00`)
 * **Purpose:** A 32-bit register controlling the I/O mode (Input, Output, Alternate Function, or Analog) for Port A pins. Each pin relies on a 2-bit wide configuration field. Pin PA5 maps to bits 10 and 11.
 * **Operation:** To configure PA5 as a General Purpose Output (01 binary), the existing configuration must first be cleared using an inverted mask before setting the target bits:
 ```c
    GPIOA_MODER &= ~(0x3 << 10); // Clears bits 10 and 11
    GPIOA_MODER |= (0x1 << 10);  // Sets bit 10 to 1, leaving bit 11 at 0
 ```

* **`GPIOA_ODR`**(Offset: `0x14`)
 * **Purpose:** The Output Data Register tracks and drives the digital logic states (3.3V High or 0V Low) of pins explicitly set as outputs.
 * **Operation:** Toggling bit position 5 drives the physical state of the pin:
 ```c
    GPIOA_ODR |= (1 << 5);   // Drive PA5 HIGH (LED ON)
    GPIOA_ODR &= ~(1 << 5);  // Drive PA5 LOW  (LED OFF)
 ```

* **`GPIOB_MODER`**(Offset: `0x00`)
 * **Purpose:** Controls the I/O mode for Port B pins. Pin PB0 maps to bits 0 and 1.
 * **Operation:** Configures PB0 as an Input (00 binary):
 ```c
    GPIOB_MODER &= ~(0x3 << 0); // Clears bits 0 and 1
 ```

* **`GPIOB_IDR`**(Offset: `0x10`)
 * **Purpose:** Tracks the incoming digital voltage states of input pins on Port B.
 * **Operation:** Samples bit 0, inverting the evaluation logic because the hardware button circuit is wired as Active-Low:
 ```c
    uint8_t raw_input = !((GPIOB_IDR >> 0) & 1);
 ```

## 🚀 How to Build and Run
The build ecosystem utilizes GNU Make and the `arm-none-eabi-` toolchain utility suite.

1. Build Compilation & Binary Linking
Compile the workspace source files and extract the raw, un-containerized machine instructions into a flat binary configuration file:
```bash
make
```

2. Flashing the Hardware
Ensure your hardware debug platform tool matches your development workspace. Clean targets exist to flash via either **ST-LINK** utilities or basic **st-flash**:
* **Option A:** Using ST-LINK Utility (Default CLI configuration):
```bash
make flash
```

* **Option B:** Using st-flash utility wrapper tools:
```bash
make flash-st
```

3. Clear Build Artifacts
To wipe cross-compilation target objects (.elf or .bin) before executing a clean environment rebuild, use:
```bash
make clean
```

## 📊 Expected Behavior
* While the physical button is left untouched, the input state floats high, filtering out accidental ground spikes, and the external LED on pin PA5 remains OFF.
* Pressing down on the push-button shorts the trace to ground. The algorithmic debouncer sample routine parses the logic state changes across its execution cycles to filter noise.
* Once a debounced press validation event is evaluated by the state machine API, the LED outputs a blink.

## 🗺️ Next Steps & Learning Roadmap
* **Hardware Timers:** Replace the blocking software busy-wait loop with a precise, deterministic internal `SysTick` timer interrupt handler.
* ☑️ **External Input Handling:** Configure Port B registers to sample a mechanical push-button switch with digital software debouncing logic.
* **Serial Communication:** Implement a basic asynchronous peripheral driver (UART) to transmit debugging frames directly to a host terminal.

## 🪪 Author & License
* **Author:** Dmytro Krapyvianskyi
* **License:** This project is open-source software licensed under the MIT License.
