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