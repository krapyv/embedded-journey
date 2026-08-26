# Low-power modes test harness
The project handles implementation of three low-power modes: Sleep, Stop and Standby via interactive UART RX menu. Sleep wakes on USART2 RXNE interrupt (key '0' pressed), Stop uses EXTI0 interrupts and Standby wakes on WKUP0 (both Stop and Stanby wake via tactile button on PA0). All three modes print confirmation over UART.

## Project Structure

```text
reusable_drivers/
├── core/   # ARM Cortex-M4 and STM32F411 register definitions
│     ├── stm32f411.h     # Memory boundaries and register definitions for AHB/APB peripherals
│     └── core_cm4.h      # Register layout definitions for NVIC and SysTick architectures
├── periph/     # Portable peripheral drivers
│    ├── uart.c      # USART2 register configuration and DMA1 Stream 6 transfer invocation
│    ├── uart.h      # USART2 bit definitions, control macros, and function
│    ├── systick.c   # SysTick initialization, counter variable and functions
│    └── systick.h   # SysTick mode, BRR, clock and register configuration and function headers
├── utils/     # Additional helpers
│    ├── ring_buffer.c   # Ring buffer initialization and control functions
│    └── ring_buffer.h   # Ring buffer control struct and function declarations
│
└── main.c      # Application test harness: low power mode functions, UART menu

```

## Architectural decisions

1. Standby reset checking at the beginning of the main function

Since the Standby mode on wake up requires the full board reset, the program does not continues after `__WFI()` exit in the 'process_standby_mode' function. It re-enters the main(). And since the SRAM and all the registers except for the PWR->CSR are reset as well, the only way to recognize the board went through the Standby is to check the CSBF bit at the beginning of the main() and set the flag if it did so.

2. Both the Stop and Standby modes uses the PA0 for wake-up

The Stop mode uses the PA0 for the EXTI0 interrupt on falling edge (the tactile button with a pull up resistor), while the Standby mode wakes on the rising edge of the WKUP 0 that also the pin PA0. The user presses and releases the button, the wake occurs. 

3. HSI clock settling time before the clock output is trustworthy

Both the Stop and Standby modes halt the HSI oscillator. After the Stop mode exits the execution continues after the `__WFI()`, meanwhile after the Standby mode exits the main() is re-entered. 
After the Stop-mode `__WFI()` returns, HSI has to start from a cold stop. The printf() executes immediately after `__WFI()` returns before HSI has had time to fully stabilize and before USART2's f_CK is running at its true, correct frequency. The first few bits transmitted while the clock is still ramping up would come out at the wrong effective baud rate - garbled - and then self-correct once the clock settles.
Now after the Stop's `__WFI()` and unconditionally at the beginning of the main() there is a timeout of 128 iterations (HSI oscillator startup time is max 4 microseconds). It fixes the HSI oscillator problem completely.

## Known Limitations

1. Both Stop and Standby modes use the same PA0.

While Stop mode utilizes the EXTI0 interrupt on the falling edge (pull up tactile button), the Standby mode uses the WKUP0 on the rising edge. There should be two separate buttons at two different pins.

2. A button press consumption before WFI()
 
 A button press occurring after menu selection but before WFI() is reached in process_stop_mode() is silently consumed by EXTI0_IRQHandler (PR cleared, no flag/confirmation).
The CPU proceeds into Stop Mode and requires a second press to wake.
Window is ~microseconds, not reachable in practice during manual demo/testing.
Not a race, bus hang, or correctness violation - the mode still wakes and confirms correctly on the press that matters.
Deferred; would require a second tracking flag and pre-critical-section tagging to resolve, disproportionate to Week 19 scope.
