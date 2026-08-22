#include "core_cm4.h"
#include "uart.h"
#include "systick.h"
#include "ring_buffer/ring_buffer.h"

extern RingBuffer_t rx_buffer;

uint8_t processed_button;
uint8_t exit_button_pressed = 0;

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
    PWR->
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
    }
}