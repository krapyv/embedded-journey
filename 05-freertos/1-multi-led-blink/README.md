# FreeRTOS Multi-LED Blink Logger

The project toggles 3 LEDs: on-board PC13 and two external PA1 and PA10, using three LED tasks with three different priorities. Every 1000 ms the logger prints the count of ticks since vTaskStartScheduler was called as well as the LEDs' counters.
It sits on top of the existing USART and LED drivers. 
The FreeRTOS port uses SysTick as the kernel tick (`configTICK_RATE_HZ` = 1000), so the SysTick driver isn't used.

## Periods and priorities

`configMAX_PRIORITIES` = 5.

| Task | Priority | Interval |
| --- | --- | --- |
| Logger | 1 (the lowest)| 1000 ms |
| PC13 | 2 | 500 ms |
| PA1 | 3 | 1200 ms |
| PA10 | 4 (the highest) | 1700 ms |

## Project Structure

```text
third_party/FreeRTOS-Kernel/
├── tasks.c     # Task creation, scheduling, and lifecycle management
├── list.c      # Circular, doubly linked list implementation
├── queue.c     # Inter-task communication and synchronization primitives implementation, FIFO queues
├── portable/GCC/ARM_CM4F/     # Hardware- and compiler-specific FreeRTOS port directory for ARM Cortex-M4F
│    └── port.c      # Low-level hardware functions implementation: SysTick timer, PendSV and SVC interrupt handlers configuration, FPU register context saving management
│
├── portable/MemMang/     # Dynamic memory management sample implementations
│    └── heap_4.c      # Dynamic memory allocator, first-fit algorithm that merges adjacent free memory blocks to reduce fragmentation
│
1-multi-led-blink/
├── config/     # Project-specific kernel configuration
│    └── FreeRTOSConfig.h      # Kernel customization, feature inclusion/exclusion, memory and hardware settings
│
├── main.c      # Application entry point: static LED instances, tasks: vToggleLed (uses vTaskDelay), vToggleLedHighest (uses xTaskDelayUntil), vPrintLogger, main initialization
├── app_config.h    # The header file for UART2 configuration selection: option UART_MODE_TX_RX_POLLING chosen
├── startup_stm32f411ceux.s     # Vector table definition, stack allocation, and Reset_Handler assembly
├── stm32f411.ld                # Linker script mapping text, data, and bss sections to FLASH and SRAM
│
reusable_drivers/
├── core/   # ARM Cortex-M4 and STM32F411 register definitions
│    ├── stm32f411.h     # Memory boundaries and register definitions for AHB/APB peripherals
│    └── core_cm4.h      # Register layout definitions for NVIC and SysTick architectures
│
├── devices/led/
│    ├── led.c     # Init function and two sets of ON/OFF functions: with ODR register use (for the existing projects) and BSRR register use (for this and the future projects)
│    └── led.h      # Handle struct and function headers
│
└── periph/     # Portable peripheral drivers
    ├── uart.c      # USART2 register configuration and DMA1 Stream 6 transfer invocation
    └── uart.h      # USART2 bit definitions, control macros, and function declarations
```

## Architectural Decisions

1. `SVC_Handler`, `PendSV_Handler` and `SysTick_Handler` mapped to the port's handlers through `#define`s in `FreeRTOSConfig.h`.
2. `configMINIMAL_STACK_SIZE` = 512 words and `configTOTAL_HEAP_SIZE` = 16384 B.
512 words = 2048 bytes and 16384 bytes = 4096 words. 
(4 application tasks + idle) * (2048 B stack + 84 bytes of TCB) = approx. 10.7 KB, so the 16384 B is enough.
The reason for a large stack: `snprintf` in newlib uses a lot of stack.
3. No peripheral ISRs: USART TX is polled, so no ISR calls the FreeRTOS API and `configMAX_SYSCALL_INTERRUPT_PRIORITY` does not constrain anything yet.
4. Only the logger task touches the UART, so `printf` needs no mutex. The LED tasks never print.
5. No task is starved because every task blocks in `vTaskDelay`/`xTaskDelayUntil` and none runs continuously.
6. `vTaskDelay` vs `xTaskDelayUntil`. `vTaskDelay` waits relative to when it's called, so the period is the delay plus execution time and it drifts. `xTaskDelayUntil` keeps a fixed period. 
`vToggleLedHighest` and `vPrintLogger` use `xTaskDelayUntil`, vToggleLed - `vTaskDelay`.
The goal was to have both delay APIs working in one project, and to have one task with an exact, checkable time grid. PA10 was the natural place for that. Additionally, the logger was the case where the grid mattered most, so it used `xTaskDelayUntil` as well.

## Test Results

Tick 436 000 -> 467 000 (31 s):

|#| Full period | Expected | Measured|
| --- | --- | --- | --- |
| Tick step | - | 1000 | 1000 every line |
| PC13 | 1.0 s | 31 | 437 -> 468 = 31 |
| PA1 | 2.4 s| approx. 12.9 | 182 -> 195 = 13 |
| PA10 | 3.4 s | approx. 9.1 | 129 -> 138 = 9 |

It ran over 7 minutes with no assert or fault.

## Breadboard circuit

There are three LEDs: PC13 (the Blackpill on-board LED), PA1 (red LED), PA10 (green LED).

1. PC13 blue LED: active-low. Writing 0 turns it on, 1 - turns off. Inverted relative to the external LEDs.

2. PA1 red LED:
- STM32F411 Pin PA1: Connected to a row sharing one terminal of a 220 Ohms Resistor.
- Resistor (Terminal 2): Connected to the Anode (longer leg) of the LED.
- LED Cathode (shorter leg): Connected to a common ground rail (by a black wire) tied directly to the GND pin of the Blackpill.

3. PA10 green LED:
- STM32F411 Pin PA10: Connected to a row sharing one terminal of a 220 Ohms Resistor.
- Resistor (Terminal 2): Connected to the Anode (longer leg) of the LED.
- LED Cathode (shorter leg): Connected to a common ground rail (by a black wire) tied directly to the GND pin of the Blackpill.

4. The UART to USB adapter:
- GND is connected to ground rail (black wire).
- TXD is connected to PA3 of the Blackpill (RX2) by a blue wire. 
- RXD is connected to PA2 of the Blackpill (TX2) by a yellow wire.

IMPORTANT: the UART-to-USB adapter's VCC pin SHOULD be left unconnected. The Blackpill itself has the USB powering, so with the UART also having a connection to the 3.3V with the STM32, there are two power supplies with different or unregulated voltages fighting to drive the same rail. The STM32F411's 3.3V rail is being driven by its onboard regulator (fed from USB 5V -> 3.3V), and the UART adapter's VCC pin is also trying to source 3.3V onto that same rail, there are two low-impedance voltage sources both trying to set the same node - the one with a slightly higher effective voltage will attempt to push current backward into the other's regulator output. Some regulators can be damaged over time or immediately, depending on current and design margin.

## How to run

Clone the repository, navigate to the project directory, and execute the toolchain commands:

```bash
# Clean previous build artifacts and compile the firmware binary
make clean
make all

# Flash the binary to the microcontroller using ST-LINK
make flash

# Check what name the UART-to-USB adapter has
ls /dev/ttyUSB* /dev/ttyACM* /dev/serial/by-id/* 2>/dev/null

# Open an UART serial communication session using minicom
minicom -D /dev/ttyUSB0 -b 115200
```

Expected minicom output:
```text
436000 | PC13: 437 | PA10: 129 | PA1: 182
437000 | PC13: 438 | PA10: 129 | PA1: 183
438000 | PC13: 439 | PA10: 129 | PA1: 183
439000 | PC13: 440 | PA10: 130 | PA1: 183
440000 | PC13: 441 | PA10: 130 | PA1: 184
441000 | PC13: 442 | PA10: 130 | PA1: 184
442000 | PC13: 443 | PA10: 131 | PA1: 185
```

## Known Limitations

1. Stack high-water marks aren't measured.
2. The kernel uses dynamic allocation (`heap_4`), not static (`configSUPPORT_STATIC_ALLOCATION`).
3. USART TX is polled: the logger busy-waits on TXE for each line instead of blocking, so that time is taken from the idle task only.
4. The logger reads each LED counter and its last-toggle tick as two separate loads, so a higher-priority LED task can preempt between them and the printed pair can come from different moments (off by one toggle). Atomic loads of each variable don't fix this, because the invariant spans two variables. A short critical section (`taskENTER_CRITICAL`/`taskEXIT_CRITICAL`) around the pair read would.

