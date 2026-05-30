# UART2 Polling Transmitter

This project provides a highly optimized, bare-metal asynchronous serial transmitter driver for the USART2 peripheral on STM32 microcontrollers, integrated alongside a non-blocking SysTick timebase module. Operating completely independent of vendor hardware abstraction layers (HAL), the implementation handles low-level peripheral clock gating, alternate function I/O multiplexing, and precise baud rate generation using raw register manipulation and direct pointer definitions.
The system utilizes non-blocking, delta-time execution loops to transmit periodic telemetry streams over a serial interface without freezing core CPU operation.

## 📂 Project Structure

```text
├── transmitter.c       # USART2 and SysTick driver implementations with test execution loop
├── transmitter.h       # Low-level memory map base definitions and function prototypes
├── startup_stm32f411ceux.s       # Assembly startup file implementing vector table & Reset_Handle   
├── stm32f411.ld        # Linker script defining Flash/SRAM memory segments
└── Makefile            # GNU Make automation configuration to compile and flash the system
```

## 🧠 Core Low-Level Concepts
### Memory Gating and Bus Routing

Before accessing peripheral control structures, their respective clock domains must be explicitly enabled inside the Reset and Clock Control (RCC) block. Because different peripherals live on separate internal bus matrices, this driver interfaces across two distinct boundaries:

* **Advanced High-Performance Bus 1 (AHB1):** Hosts high-speed digital I/O structures including `GPIOA` (enabled via bit 0 of `RCC_AHB1ENR`).
* **Advanced Peripheral Bus 1 (APB1):** Hosts lower-speed communication primitives including `USART2` (enabled via bit 17 of `RCC_APB1ENR`).

### Alternate Function I/O Multiplexing

To round internal peripheral signals to physical silicon breakout pins, the GPIO pins must be explicitly detached from the standard General Purpose Input/Output latch and tied to the internal Alternate Function (AF) multiplexer matrix.

1. **Mode Allocation (`GPIOx_MODER`):** Pins `PA2` (TX) and `PA3` (RX) are allocated to Alternate Function mode by assigning the binary value `10` across their respective 2-bit channel blocks.
2. **Multiplexer Routing (`GPIOx_AFRL`):** The driver configures the low-register Alternate Function matrix (`AFRL`) to assign **AF7** (`0b0111`), explicitly mapping the physical pins directly to the USART2 TX and RX peripheral lines.

## 📐 Mathematical Modeling & Register Architecture

### Baud Rate Engine Calculation

The USART baud rate is derived dynamically from the frequency of the underlying peripheral bus clock ($f_{CK}$) divided by a programmable scaling factor managed inside the Baud Rate Register (`USART2_BRR)`.

The system equation for the Baud Rate is:

$$\text{Baud Rate} = \frac{f_{CK}}{8 \times (2 - \text{OVER8}) \times \text{USARTDIV}}$$

Given standard target parameters for this implementation:
* $f_{CK}$ = 16,000,000 Hz (Internal High-Speed Oscillator clock frequency)
* OVER8 = 0 (Default 16x oversampling mode selected)
* Target Baud Rate = 115,200 bps

$$\text{USARTDIV} = \frac{16{,}000{,}000}{16 \times 115{,}200} = \mathbf{8.68055}$$

To translate this floating-point scaling multiplier cleanly into the split integer/fraction bitfields of `USART2_BRR`:
* **Mantissa (Integer Part):** 8 = **0x8**
* **Fraction Part:** 0.68055 x 16 = 10.888 approx.= 11 = **0x0B**

Combining these parameters into a unified 16-bit payload yields the register assignment code:

```c
USART2_BRR = 0x008B;
```

### Non-Blocking SysTick Timebase Tracking

Instead of executing classic variable-burning `delay()` loops that block the CPU core from performing asynchronous application routines, the timebase relies on a 24-bit down-counting Cortex-M system timer (`SysTick`).

The Reload Value Register (`SYST_RVR`) is scaled according to the processor's core operating frequency to trigger a dedicated hardware vector exactly once every 1 millisecond:

$$\text{Reload Value} = \left( \frac{f_{CPU}}{\text{Desired Interrupt Frequency}} \right) - 1$$

$$\text{Reload Value} = \left( \frac{16{,}000{,}000\text{ Hz}}{1{,}000\text{ Hz}} \right) - 1 = \mathbf{15{,}999}$$

The `SysTick_Handler` interrupt increments a tracking counter (`ms_passed`), enabling non-blocking delta-time evaluations inside the background processing layer:

```c
if (ms_passed - last_transmission >= 500) {
    // Execute communication task safely every 500 ms
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
To wipe cross-compilation target objects (`.elf` or `.bin`) before executing a clean environment rebuild, use:
```bash
make clean
```

4. Connect a USB-to-UART terminal bridge to pins `PA2` and `PA3` and open your preferred host monitoring software (e.g., `minicom` that I use) at **115200 baud** to monitor transmission logs.

```text
--- Hardware Terminal Monitoring Initiated ---
Hello, World!
Hello, World!
Hello, World!
```

## 🗺️  Next Steps & Learning Roadmap

To transition this basic communication driver into a standard automotive-compliant software architecture component, future developments will introduce:

1. **Interrupt-Driven Asynchronous Transmission:** Move away from busy-polling the `TXE` status flag by implementing internal transmit queues that trigger automatically via hardware interrupts.
2. **DMA (Direct Memory Access) Interfacing:** Integrate the system stream with an active DMA controller channel, allowing high-throughput multi-byte string transfers to pipe directly from RAM to the peripheral output buffer without consuming CPU processing cycles.
3. **Parity and Frame Error Diagnostics:** Expand register initializations to enforce standard hardware error boundaries (e.g., framing errors, overrun monitoring, and parity bit insertion for noisy interconnect buses).

## 🪪 Author & License
* **Author:** Dmytro Krapyvianskyi
* **License:** This project is open-source software licensed under the MIT License.
