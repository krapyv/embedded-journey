# LED Blink - STM32F411CEU6 Bare Metal

A lightweight, bare-metal C application that blinks an external LED connected to pin PA5 on the STM32F411 (Blackpill) development board. This implementation utilizes a custom startup script and a tailored linker configuration without relying on standard libraries, Vendor HAL layers, or external runtime frameworks. 

## 📋 Hardware Connections
The circuit was constructed on an 830-point breadboard using the following schematic configuration:
* **STM32F411 Pin PA5**: Connected to a row sharing one terminal of a **220 Current-Limiting Resistor**.
* **Resistor (Terminal 2)**: Connected to the **Anode (longer leg)** of the LED.
* **LED Cathode (shorter leg)**: Connected to a common ground rail tied directly to the **GND pin of the Blackpill**.

## 📂 Project Structure

```text
├── led_blink.c       # Main application logic and busy-wait delay loop
├── stm32f411.ld      # Linker script defining Flash/SRAM memory segments
└── startup_stm32f411ceux.s  # Assembly startup file implementing vector table & Reset_Handler
```

## 🧠 Core Low-Level Concepts
### Memory Map Base Addresses
* **`RCC_BASE`** (`0x40023800`): Base memory address for the Reset and Clock Control peripheral.
* **`GPIOA_BASE`** (`0x40020000`): Base memory address for General Purpose I/O Port A registers.

### Register Configuration & Bitwise Operations

* **`RCC_AHB1ENR`**(Offset: `0x30`)
 * **Purpose:** Enables peripheral clock gating for devices connected to the Advanced High-Performance Bus 1 (AHB1). Peripherals remain unclocked by default to minimize power consumption.
 * **Operation:** Bit 0 maps directly to GPIOA. Setting this bit via a bitwise OR mask activates the port:
 ```c
    RCC_AHB1ENR |= (1 << 0);
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

## 🚀 How to Build and Run
Follow these terminal steps to compile the application and flash the device using the cross-compilation toolchain and the built-in DFU bootloader.

1. Cross-Compilation:
Compile the raw assembly startup routine and C source code into a target-specific ELF binary matching the Cortex-M4 architecture:
```bash
arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -nostdlib -nostartfiles -T stm32f411.ld startup_stm32f411ceux.s led_blink.c -o blink.elf
```

2. Binary Extraction:
Strip the debugging symbols and metadata out of the ELF file to extract the raw machine instructions into a flat binary file:
```bash
arm-none-eabi-objcopy -O binary blink.elf blink.bin
```

3. Flashing to Board via DFU Bootloader:
Put the Blackpill into DFU mode (hold the BOOT0 button, press NRST, then release BOOT0). Execute dfu-util to load your executable directly into Flash internal memory space at 0x08000000:
```bash
dfu-util -a 0 -s 0x08000000:leave -D blink.bin
```

## 🗺️ Next Steps & Learning Roadmap
* **Hardware Timers:** Replace the blocking software busy-wait loop with a precise, deterministic internal `SysTick` timer interrupt handler.
* **External Input Handling:** Configure Port B registers to sample a mechanical push-button switch with digital software debouncing logic.
* **Serial Communication:** Implement a basic asynchronous peripheral driver (UART) to transmit debugging frames directly to a host terminal.

## 🪪 Author & License
* **Author:** Dmytro Krapyvianskyi
* **License:** This project is open-source software licensed under the MIT License.
