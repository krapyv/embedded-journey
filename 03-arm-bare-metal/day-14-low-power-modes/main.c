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
        SCB->SCR &= ~((1 << 4) | (1 << 2));

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

    // bit 4 SEVONPEND - set to 0 to ensure that only enabled interrupts or events can wakeup the processor, disabled interrupts are excluded
    SCB->SCR &= ~(1UL << 4U);

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

void main(void)
{
    usart2_init();

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
    }
}