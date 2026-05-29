# Button-Controlled LED Blink - STM32F411CEU6 Bare Metal

A lightweight, bare-metal C application that tracks interaction via a tactile push-button on pin PB0 to control an external LED connected to pin **PA5** on the STM32F411 (Blackpill) development board.
This repository implements a complete low-level driver stack without relying on standard vendor libraries or external HAL frameworks.
Button noise filtering is handled via a dedicated algorithmic software state-machine debouncer running inside a non-blocking execution window driven by the ARM Cortex-M `SysTick` peripheral.

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
├── led_blink.c       # Application entry point, SysTick timebase configuration, and polling loop
├── led_blink.h       # Macro layouts and register maps
├── button_debouncer.c      # Algorithmic state-machine debouncer logic implementation
├── button_debouncer.h      # Debouncer states, thresholds, and abstract APIs
├── stm32f411.ld      # Linker script defining Flash/SRAM memory segments
├── startup_stm32f411ceux.s  # Assembly startup file implementing vector table & SysTick_Handler 
└── Makefile         #GNU Make automation configuration to compile and flash the system    
```

## 🧠 Core Low-Level Concepts
### Memory Map Base Addresses
* **`RCC_BASE`** (`0x40023800`): Base memory address for the Reset and Clock Control peripheral.
* **`GPIOA_BASE`** (`0x40020000`): Base memory address for General Purpose I/O Port A registers.
* **`GPIOB_BASE`** (`0x40020400`): Base memory address for General Purpose I/O Port B registers.
* **`SCS_BASE`** (`0xE000E010`): System Control Space base address hosting the core `SysTick` register group.


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

## ⏱️ Deterministic Hardware Timebase: ARM SysTick

Instead of relying on unstable software busy-loops (`for` loops counting to arbitrary numbers), this driver establishes a deterministic, hardware-driven millisecond timebase utilizing the internal **ARM Cortex-M SysTick Timer**. This core peripheral acts as a dedicated 24-bit down-counter that generates periodic hardware exceptions directly managed by the processor core.

### 1. Clock Configuration & Timing Calculations

The application operates on the default internal high-speed oscillator (`HSI`) running at a fundamental clock frequency ($f_{CPU}$) of **16 MHz**. To configure a precise **1 millisecond (1 ms)** interrupt interval, the timer's reload register must be calculated using the following structural model:

$$\text{Reload Value } (\text{RVR}) = (\text{Clock Frequency } \times \text{Target Interval}) - 1$$

$$\text{RVR} = (16{,}000{,}000\text{ Hz} \times 0.001\text{ s}) - 1 = 16{,}000 - 1 = \mathbf{15{,}999}$$

```c
SYST_RVR = 15999; // Configures down-counter reset limit for a 1 ms period
SYST_CVR = 0; // Clears the current register to force an immediate reload
```

### 2. Control Register Activation Matrix

To activate the counter and unmask its underlying interrupt vector line, three distinct bits within the SysTick Control and Status Register (`SYST_CSR`) are written simultaneously:

| Bit Position | Configuration Flag Name | Value | Explicit Hardware Impact |
| ------------ | ----------------------- | ----- | ------------------------ |
| **Bit 0** | `ENABLE` | `1` | Powers on and enables the counter mechanism |
| **Bit 1** | `TICKINT` | `1` | Asserts a hardware exception vector when the counter down-counts to 0 |
| **Bit 2** | `CLKSOURCE` | `1` | Selects the raw processor core clock (16 MHz HSI) rather than external scaling |

```c
 // flipping 3 bits: 2 - CLKSOURCE to 1 (processor clock), 1 - TICKINT to 1 (to allow interrups), 0 - ENABLE to 1 (to enable the counter)
SYST_CSR |= (1 << 2) | (1 << 1) | (1 << 0);
```

### 3. Asynchronous ISR Management

When the counter crosses zero, the hardware preempts the main runtime execution thread, jumping to position **15** within the vector table mapped in the assembly startup file (`startup_stm32f411ceux.s`). This location executes the C function `SysTick_Handler()`, incrementing a volatile global accumulator:

```c
volatile uint32_t ms_passed = 0;

void SysTick_Handler(void)
{
    ms_passed++;
}
```

### 4. Non-Blocking State Machine Execution

Rather than polling the hardware input as fast as the CPU allows, the system uses a delta-time evaluation structure to instantiate a precise **100 ms execution window** for the state-machine debouncer update loop. This ensures that the debouncer updates at a fixed, predictable rate regardless of the density of your main thread code:

```c
 if (ms_passed - last_debounce_call >= 100) {
    uint8_t raw_input = !((GPIOB_IDR >> 0) & 1);
    uint8_t button_pressed = button_debouncer_update(&button_debouncer, raw_input, 10);
    ...

    last_debounce_call = ms_passed;
}
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
* Pressing down on the push-button shorts the trace to ground. The algorithmic debouncer sample routine parses the logic state shifts inside a strict **100 ms non-blocking scheduling window**.
* Once a debounced press validation event is evaluated by the state machine API, the LED outputs a blink.

## 🗺️ Next Steps & Learning Roadmap
* ☑️ **Hardware Timers:** Replace the blocking software busy-wait loop with a precise, deterministic internal `SysTick` timer interrupt handler.
* ☑️ **External Input Handling:** Configure Port B registers to sample a mechanical push-button switch with digital software debouncing logic.
* **Serial Communication:** Implement a basic asynchronous peripheral driver (UART) to transmit debugging frames directly to a host terminal.

## 🪪 Author & License
* **Author:** Dmytro Krapyvianskyi
* **License:** This project is open-source software licensed under the MIT License.
