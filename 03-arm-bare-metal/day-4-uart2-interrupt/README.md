# USART2 Interrupt-Driven Receiver & Reusable Subsystem

This project implements a highly optimized, asynchronous, interrupt-driven serial receiver for the STM32 `USART2` peripheral. It operates completely bare-metal without relying on vendor hardware abstraction layer (HAL). The system integrates a global Interrupt Service Routine (ISR) mapped through the Nested Interrupt Controller (NVIC) alongside a previously developed ring buffer library to process incoming bytes without dropping data or blocking execution.
The main execution thread features a non-blocking command parser that evaluates incoming telemetry and toggles peripheral hardware states via a critical section, balanced with a `SysTick`-driven periodic transmitter loop.

## 📂 Project Structure

```text
├── main.c        # Thread loop, critical section shielding, and basic command parser
├── uart2.c       # USART2 configuration, clock gating, and ISR implementation
├── uart2.h       # Core hardware memory map, peripheral offsets, and NVIC flags
├── ring_buffer.c # Reusable thread-safe-conscious circular queue component
├── ring_buffer.h # Circular buffer status structures and pointer tracking
├── led.c         # Reusable bare-metal GPIO output driver  
├── led.h         # LED state handle abstractions and register maps
├── startup_stm32f411ceux.s       # Assembly startup file implementing vector table & Reset_Handle   
├── stm32f411.ld  # Linker script defining Flash/SRAM memory segments
└── Makefile      # GNU Make automation configuration to compile and flash the system
```

## 🧠 Core Low-Level Concepts
### Asynchronous Interrupt Routing & Vector Mapping

When a byte is received over the RX line, the `USART2` peripheral asserts a hardware interrupt line routed directly to the Nested Vectored Interrupt Controller (NVIC). To handle this event asynchronously:
1. **Vector Table Mapping:** The assembly startup script (`startup_stm32f411ceux.s`) maps position **38** of the processor's exception vectors to the symbol `USART2_IRQHandler`.
2. **NVIC Register Selection:** The interrupt offset calculation determines which hardware activation latch to target:

$$\text{Register Index} = \lfloor \frac{\text{Vector Position}}{32} \rfloor = \lfloor \frac{38}{32} \rfloor = \mathbf{1}$$

$$\text{Bit Position} = \text{Vector Position} \pmod{32} = 38 \pmod{32} = \mathbf{6}$$

Setting bit 6 within `NVIC_ISER1` (Interrupt Set-Enable Register) unmasks the channel at the core processor boundary.

### Atomic Critical Sections & Race Conditions

Because the `USART2_IRQHandler` can preempt the background thread execution loop at any arbitrary clock cycle, reading or writing to the shared `rx_buffer` component simultaneously would cause severe data structure corruption (e.g., intermediate head/tail pointer misalignment). 

To prevent this race condition, the driver creates a strict **Critical Section** inside `main.c` by temporarily disabling the interrupt channel before evaluating the buffer pointers, and restoring it immediately after data extraction:

```c
NVIC_ICER1 = (1 << 6); // Atomic Write-1-to-Clear, disables USART2 preemption
uint8_t data_available = ring_buffer_pop(&rx_buffer, &processed_char);
NVIC_ISER1 = (1 << 6);
```

**Note:** Using a direct assignment (`=`) instead of a read-modify-write bitwise OR operation (`|=`) on `NVIC_ICERx` and `NVIC_ISERx` is structurally required. These registers are implemented as dedicated hardware latches where writing a zero has no effect, ensuring atomic single-cycle adjustments without timing race windows.

## 🛠️ Register Diagnostics & Error Resilience

The driver's `USART2_IRQHandler` acts as a first-line hardware diagnostic manager, reading the Status Register (`USART2_SR`) snapshot into a temporary stack variable to evaluate execution states efficiently within a single bus cycle:

### 1. RXNE (Read Data Register Not Empty) Check

If bit 5 (`RXNE`) is set, incoming data has crossed the deserializer shift register and is ready for reading inside the Data Register (`USART2_DR`). The driver reads the payload and performs an explicit 8-bit truncation:

```c
uint8_t temp = (uint8_t)USART22_DR;
```

### 2. Error Mitigation Loops (FE and ORE)

* **Frame Error (`FE`, Bit 1):** Triggered if a stop bit is not recognized due to line noise or synchronization mismatch. The driver filters out corrupt framing fragments before committing the byte to memory.

* **Overrun Error (`ORE`, Bit 3):** Triggered if a new byte arrives before the software reads the previous byte out of `USART2_DR`.

According to the reference manual, clearing an `ORE` condition requires a specific hardware sequence: reading the `USART2_SR` register followed by reading the `USART2_DR` register. The handler performs this sequence atomically to clear the error state and prevent the peripheral from freezing up:

```c
if (sr_snapshot & ((1 << 3) | (1 << 1))) {
    if (!(sr_snapshot & (1 << 5))) {
            volatile uint32_t clear = USART2_DR;
            (void)clear;
    }
}
```

## 🚀 How to Build and Validate
The software stack utilizes standard cross-compilation tools and host terminal multiplexers.

### 1. Build and Flash Pipeline
Compile the entire project using your automated GNU Make rules and load the binary file onto your target microcontroller:

```bash
make
make flash
```

### 2. Live Runtime Control Validation
Connect a USB-to-UART terminal device to the microcontroller breakout boardpins (`PA2` for TX, `PA3` for RX). Initialize a host terminal utility at a standard communication speed of **115200 baud** (e.g., using `minicom`):

```bash
minicom -D /dev/ttyUSB0 -b 115200
```

### Verification Terminal Stream:

```text
System Status: OK
System Status: OK
[User Transmits '1' via host keyboard] -> Hardware Pin PA5 LED turns ON
System Status: OK
[User Transmits '0' via host keyboard] -> Hardware Pin PA5 LED turns OFF
System Status: OK
```

## 🗺️  Next Steps & Learning Roadmap

To build on this asynchronous framework for larger automotive or real-time application systems, the upcoming technical roadmap focuses on:

1. **Interrupt-Driven Non-Blocking Transmission:** Upgrade the transmitter from busy-polling the `TXE` flag to utilizing a second transmission ring buffer. This buffer will stream text strings in the background using the Transmit Data Register Empty interrupt channel.
2. **Double-Buffered DMA Interfacing**: Route serial data directly through a Direct Memory Access channel operating in a circular buffer layout, removing the CPU entirely from the byte-by-byte ISR handling path.
3. **Hardware Frame Line Filtering**: Implement hardware parity bits and line-idle tracking variables to support multiprocessor communication messaging networks on shared communication channels

## 🪪 Author & License
* **Author:** Dmytro Krapyvianskyi
* **License:** This project is open-source software licensed under the MIT License.
