#include <stdio.h>
#include "core_cm4.h"
#include "stm32f411.h"
#include "uart.h"
#include "systick.h"
#include "ring_buffer/ring_buffer.h"

extern RingBuffer_t rx_buffer;

uint8_t processed_button;
uint8_t exit_button_pressed = 0;

// TODO: a button press occurring after menu selection but before WFI() is reached in process_stop_mode() is silently consumed by EXTI0_IRQHandler (PR cleared, no flag/confirmation)
// the CPU proceeds into Stop Mode and requires a second press to wake.
// Window is ~microseconds, not reachable in practice during manual demo/testing.
// Not a race, bus hang, or correctness violation - the mode still wakes and confirms correctly on the press that matters.
// Deferred; would require a second tracking flag and pre-critical-section tagging to resolve, disproportionate to Week 19 scope.
void EXTI0_IRQHandler(void)
{
    // clear the pending EXTI0 flag
    EXTI->PR = (1UL << 0U);
}

uint8_t inspect_byte(uint8_t character)
{
    processed_button = 0;
    uint8_t data_available = ring_buffer_pop(&rx_buffer, &processed_button);

    if (data_available)
    {
        // if read "1", then go to Sleep mode
        if (processed_button == character)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    return 0;
}

void process_sleep_mode(void)
{
    exit_button_pressed = 0;
    SysTick_Init(SYSTICK_FREQUENCY_16MHZ);

    while (exit_button_pressed != 1)
    {
        /* Mode entry: WFI (Wait for Interrupt) or WFE (Wait for Event) while:
        - SLEEPDEEP = 0, and
        - No interrupt (for WFI) or even (for WFE) is pending
        */

        // SLEEPDEEP bit 2 in SCR
        // SEVONPEND bit 4 in SCR
        // SCR reset value is 0x00000000
        SCB->SCR &= ~(1 << 2);

        __WFI();

        if (inspect_byte('0'))
        {

            exit_button_pressed = 1;
        }
    }

    SYST->CSR &= ~(1UL << 0U); // disable the counter
}

void process_stop_mode(void)
{
    // enable SYSCFG and PWR clocks

    // PWR clock belongs to RCC_APB1ENR register
    RCC->APB1ENR |= (1UL << 28U);

    // SYSCFG clock belongs to RCC_APB2ENR register
    RCC->APB2ENR |= (1UL << 14U);

    // select the source input for the EXTIx external interrupt
    // PA0
    // EXTICRx takes 4 bits per pin

    // clear the bits 3:0 - 1111 = 2^3 + 2^2 + 2^1 + 2^0 = 8 + 4 + 2 + 1 = 15 = 0xF
    SYSCFG->EXTICR1 &= ~(0xFUL << 0U);

    // select the falling edge trigger for EXTI0
    EXTI->FTSR |= (1UL << 0U);

    // enable the interrupt in NVIC
    // EXTI0 interrupt has the position 6 in the vector table
    // so EXTI0 is NVIC->ISER[0] bit 6
    NVIC->ISER[0] = (1UL << 6U);

    // unmask the line 0 of EXTI
    EXTI->IMR |= (1UL << 0U);

    // configure PWR->CR register (direct write, first access)

    // bit 1 PDDS - set to 0 to enter Sleep mode when the CPU enters deepsleep
    // bit 0 LPDS - set to 1 to use low-power voltage regulator during Stop mode (~43µA current drawing)
    // 0001 = 0x01
    PWR->CR = (0x01UL << 0U);

    // configure SCB->SCR register (RMW since the Sleep mode already touched this register)

    // bit 2 SLEEPDEEP - set to 1 since the Stop Mode requires this bit to be set to 1
    SCB->SCR |= (1UL << 2U);

    // start of the critical section
    __disable_irq();

    // clear stale pending EXTI0 interrupt by programming it to 1
    // a direct write since the PR register bits are write-1-to-clear
    // 1 << 0 resolves to all 0s and the bit 0 as 1
    // so no other bit except for the bit 0 is cleared in the PR register
    // additionally, the direct write is one instruction, when the RMW is three instructions: read, modify and w
    EXTI->PR = (1UL << 0U);

    __enable_irq();
    // end of the critical section

    // immediate WFI after the critical section
    __WFI();

    printf("The CPU has woken up from the Stop mode\n");
}

/*
NOTE: The Standby mode shares the button with the Stop mode on PA0
The difference: Stop mode wakes up on falling edge (press of the button). Standby mode wakes up on rising edge (release of the button).

The internal pull-down on PA0 has min. 30 kOhms, typ. 40 kOhm.
My external pull-up resistor has 10 kOhms.

According to the Ohm's law, the external pull up resistor will win but there would be a voltage divider between two competing resistances.
The pin will see Logic 1 while idle. so the standby will wake up not on the button pressed, but button released.

Voltage divider formula: V = Vdd * Rpd / (Rpd + Rpu)
- Best case:  3.3V * 40 kOhms / 50 kOhms = 3.3 * 4/5 = 2.64V
- Worst case : 3.3V * 30 kOhms / (30 kOhms + 10 kOhms) = 3.3 * 3/4 = 2.475 V.

According to the I/O static characteristics, Vih minimum = 0.7 Vdd => 0.7 * 3.3V = 2.31 V

*/
// TODO: Move the Standby button on the different WKUP pin, wiring to the different polarity: between the pin and VDD.
void process_standby_mode(void)
{
    // enable PWR clock
    RCC->APB1ENR |= (1UL << 28U);

    // WUF clearing and PDDS setting
    // the Standby mode requires the WUF bit of CSR to be cleared before entering the mode, otherwise the _WFI is no-op
    // PDDS=1 configures that when the CPU enters deepsleep the Standby more is entered
    // LPDS bit does not matter for the Standby mode

    // direct write since only CWUF and PDDS bits matter for the Standby mode and LPDS is setting to zero. no other bits have ever been changed in any other place of the program

    // 0110 = 2^2 + 2^1 = 4 + 2 = 6 = 0x6
    PWR->CR = (0x6UL << 1U);

    // WKUP pin enablement
    PWR->CSR = (1UL << 8U);

    // configure SCB->SCR (RMW, since this register has already been modified in other places of the program)

    // bit 2 SLEEPDEEP - set to 1 since the Standby Mode requires this bit to be set to 1
    SCB->SCR |= (1UL << 2U);
    __WFI();
}

void main(void)
{
    uint8_t is_reset_after_standby = 0;

    // check whether the system reset after the Standby mode
    // enable PWR clock
    RCC->APB1ENR |= (1UL << 28U);

    if (PWR->CSR & (1UL << 1U))
    {
        is_reset_after_standby = 1;
        PWR->CR = (1UL << 3U);
    }

    usart2_init();

    if (is_reset_after_standby)
    {
        printf("The CPU has woken up from the Standby mode.\n");
    }

    while (1)
    {
        if (inspect_byte('1'))
        {
            process_sleep_mode();
        }
        else if (inspect_byte('2'))
        {
            process_stop_mode();
        }
        else if (inspect_byte('3'))
        {
            process_standby_mode();
        }
    }
}