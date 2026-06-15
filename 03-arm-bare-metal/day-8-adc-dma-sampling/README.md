# STM32F411 ADC-to-UART Data Pipeline

ADC1 Channel 0 is triggered at 10 kHz by TIM2, with samples transferred via DMA2 Stream 0 direcly to a ping-pong buffer in SRAM. The CPU only wakes on Half-Transfer (HT) and Transfer-Complete (TC) interrupts to format the data into ASCII string blocks. These blocks are streamed to a host terminal via USART2 at 921,600 baud using DMA1 Stream 6, running against a 16 MHz HSI clock with a calculated BRR error rate around 2%.

## Technical Specifications 
* **Sampling Rate:** 10 kHz driven by TIM2 TRGO
* **Bus Topology:** DMA2 Stream 0 (ADC1) -> SRAM -> CPU (ASCII conversion) -> DMA1 Stream 6 -> USART2 TX
* **Clock Domain:** 16 MHz HSI (SYSCLK/AHB/APB1/APB2)
* **Serial Configuration:** 921,600 baud. `USART->BRR = 0x11` - integer truncation of 17.36, yielding ~2.08% baud error
* **Hardware Configurability:** Compile-time macros for GPIO port selection (Port A/B), ADC sample time cycles, and circular buffer dimentions

## Project Structure

```text
├── main.c          # Application entry point, initialization sequence, and background scheduling loop
├── sampling.c      # Configures the ADC1 trigger chain and handles DMA transfer interrupts
├── sampling.h      # Shared state flags, hardware macros, and initialization API signatures
├── app_config.h    # Reusable UART driver compile-time configuration and baud-time parameters
reusable_drivers/
├── core/   # ARM Cortex-M4 and STM32F411 register definitions
    ├── core_cm4.h      # Register layout definitions for NVIC and SysTick architectures
    └── stm32f411.h     # Memory boundaries and register definitions for AHB/APB peripherals
└── periph/     # Portable peripheral drivers (UART)
    ├── uart.c      # USART2 register configuration and DMA1 Stream 6 transfer invocation
    └── uart.h      # USART2 bit definitions, control macros, and function declarations
├── startup_stm32f411ceux.s       # Vector table definition, stack allocation, and Reset_Handler assembly
├── stm32f411.ld  # Linker script mapping text, data, and bss sections to FLASH and SRAM
└── Makefile      # Compilation rules and ST-LINK flash targets using arm-none-eabi-gcc
```

## Low-Level Architectural Decisions

### Ping-Pong Buffers vs. Single Circular Buffer (Race Condition Prevention)
* **The Decision:** Memory is split into two isolated arrays (`tx_buffer_a` and `tx_buffer_b`) rather than streaming from a single circular buffer.
* **The Why:** A single buffer creates a Write-After-Read (WAR) data hazard if the CPU modifies memory while the DMA controller is still reading it to feed the USART shift register. Structural isolation ensures the CPU and the DMA engine never own the same memory region simultaneously, completely eliminating string corruption without requiring blocking delays.

### DMA2 for ADC vs. DMA1 for UART (Silicon Bus Constraints)
* **The Decision:** ADC1 is explicitly mapped to DMA2 Stream 0, while USART2 is mapped to DMA1 Stream 6.
* **The Why:** This was dictated entirely by the STM32F411 bus matrix architecture, not personal preference. On this silicon, DMA1 is not connected to the APB2 peripheral bus and physically cannot access the ADC1 data register. Conversely, DMA2 has master ports to both SRAM and APB2, allowing it to offload the high-frequency 10 kHz analog sampling without stalling the core.

### `CPSID i` / `CPSIE i` Assembly vs. Mutexes (Bare-Metal Threading)
* **The Decision:** Shared status flags (`dma_half_a_ready`, `dma_half_b_ready`) are protected by raw `PRIMASK` register manipulation via inline assembly.
* **The Why:** Without an RTOS or an underlying operating system, standard synchronization primitives like mutexes do not exist. To prevent the 10 kHz ADC interrupt from preempting the `main()` loop mid-evaluation, global interrupts are masked using `cpsid i` during critical state changes. This is paired with a Data Memory Barrier (`__DMB()`) to enforce memory access ordering, guaranteeing that the flag state modification is fully commited to SRAM and visible to the rest of the system before interrupts are re-enabled via `cpsie i`.

## Hardware Connections

| **STM32F411 Pin** | **Peripheral Function** | **Connected To** | **Signal Type** | **Notes** |
| ----------------- | ----------------------- | ---------------- | --------------- | --------- |
| **PA0** | ADC1_IN0 | Potentiometer Wiper (Central Pin) | Analog Input | Voltage range: 0V to 3.3V |
| **PA2** | USART2_TX | USB-to-UART Bridge **RX** | Digital Output | 921,600 baud serial telemetry stream |
| **3V3** | VDD | Potentiometer Outer Pin 1 | Power Supply | Provides the analog reference rail |
| **GND** | VSS | Potentiometer Outer Pin 2 / Bridge **GND** | Ground Reference | **Critical:** Common ground must link all devices |

[!WARNING]
**Common Ground:** Ensure the ground pin (`GND`) of your USB-to-UART bridge is physically linked to the ground pin of the STM32F411 board. Without a common ground, the UART receiver has no voltage reference, causing logic level misinterpretation and framing errors. Do not connect the bridge's 5V power output to the STM32 3.3V rail.

## Quick Start

### Prerequisites

Ensure your local system has the GNU ARM Embedded Toolchain and the ST-LINK utility installed and available in your system path:

```bash
arm-none-eabi-gcc --version
STM32_Programmer_CLI --version
```

### Hardware Setup

Connect your hardware components according to the terminal layout below:

```text
[ Potentiometer ]                  [ STM32F411 Black Pill ]             [ USB-to-UART Bridge ]
  Outer Pin 1 (3.3V)  <------------>  3V3
  Wiper Pin   (Signal) <------------>  PA0 (ADC1_IN0)
  Outer Pin 2 (GND)   <------------>  GND  <--------------------------->   GND
                                      PA2 (USART2_TX) <---------------->   RX
```

### Build and Flash

Clone the repository, navigate to the project directory, and execute the toolchain commands:

```bash
# Clean previous build artifacts and compile the firmware binary
make clean
make all

# Flash the binary to the microcontroller using ST-LINK
make flash
```

### Monitor Telemetry

Open `minicom` (or your preferred terminal emulator) configured to match the hardware baud rate settings:

```bash
minicom -D /dev/ttyUSB0 -b 921600
```

You should instantly see the real-time telemetry stream outputting raw potentiometer values and the cleared overrun counter status:

```text
[BUFF A] Pot: 1049 | Ovrns: 0000
[BUFF B] Pot: 1052 | Ovrns: 0000
[BUFF A] Pot: 1050 | Ovrns: 0000
```

## Known Limitations

### Real-Time Overrun Counter Accumulation

* **The Symptom:** Under sustained 10 kHz sampling, the telemetry logs show sequential, rhythmic incrementation of the `Ovrns` counter (`+1` per line).

* **The Mechanics:** The software tracks missing frames by evaluating the synchronization flags (`dma_half_a_ready` and `dma_half_b_ready`) inside the background scheduling loop. An overrun is registered when a new DMA half-transfer interrupt fires before the main loop has finished formating and clearing the previous frame's flag.

* **Current Status:** The structural bottleneck - whether it stems from APB/AHB bus matrix contention, DMA stream priority starvation, or execution latency inside the `Newlib Nano` string-formatting library under unoptimized (`-O0`) compilation - has not yet been isolated. The pipeline currently runs with this systematic delay, meaning telemetry processing lags exactly one frame behind the real-time hardware clock injection window.

##  Next Steps & Learning Roadmap

This roadmap focuses exclusively on optimizing the existing telemetry pipeline, resolving the documented runtime overruns, and extending host-side data utility.

### 1. Telemetry Bottleneck Isolation & Profiling

* **Objective:** Determine the precise source of the `Ovrns` counter incrementation.
* **Tasks:**
- Implement hardware profiling pins (toggling spare GPIOs at the entry/exit of the DMA ISR and main loop formatting blocks) to measure exact execution latency on an oscilloscope.
- Compare cycle overhead of the current custom formatting math against optimized fixed-point bit shifting to bypass `Newlib Nano` printf/division libraries.
- Benchmark the system at higher compilation optimization levels (`-O1` and `-O2`) to investigate whether Flash memory wait states at 16 MHz introduce measurable instruction fetch overhead.

### 2. Dynamically Profile Buffer Depth vs. Overhead

* **Objective:** Find the optimal balance between CPU interrupt frequency and memory utilization.
* **Tasks:**
- Test a matrix of buffer sizes (from 100 to 5000 samples) at the 10 kHz hardware sampling rate.
* Document the threshold where memory footprint efficiency intersects with CPU availability, mapping out the exact point where interrupt preemption latency or flag evaluation causes transmission back-pressure.

### 3. Host-Side Telemetry Parser & Visualizer

* **Objective:** Replace the hardcoded `minicom` terminal dependency with a flexible host-side logging system.
* **Tasks:**
- Write a lightweight Python script using `pyserial` to automatically detect, bind, and open the active USB-to-UART bridge without manual terminal configurations.
- Implement real-time string parsing to extract the `Pot` and `Ovrns` values into a structured format (CSV/JSON).
- Integrate `matplotlib` to plot the live 10 kHz analog signal curve and track the overrun accumulation over long runtime intervals.

## Author & License
* **Author:** Dmytro Krapyvianskyi
* **License:** This project is open-source software licensed under the MIT License.

