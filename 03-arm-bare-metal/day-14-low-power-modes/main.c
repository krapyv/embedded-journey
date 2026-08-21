#include "core_cm4.h"
#include "uart.h"
#include "systick.h"
#include "ring_buffer/ring_buffer.h"

extern RingBuffer_t rx_buffer;

uint8_t processed_button;
uint8_t exit_button_pressed = 0;

void process_sleep_mode(void)
{
    while (exit_button_pressed != 1)
    {
        /* Mode entry: WFI (Wait for Interrupt) or WFE (Wait for Event) while:
        - SLEEPDEEP = 0, and
        - No interrupt (for WFI) or even (for WFE) is pending
        */

        // SLEEPDEEP bit 2 in SCR
        // SEVONPEND bit 4 in SCR
        SCB->SCR = ((0 << 4) | (0 << 2));

        SysTick_Init(SYSTICK_FREQUENCY_16MHZ);

        __WFI();

        processed_button = 0;
        uint8_t data_available = ring_buffer_pop(&rx_buffer, &processed_button);

        if (data_available)
        {
            // if read "1", then go to Sleep mode
            if (processed_button == '0')
            {
                exit_button_pressed = 1;
            }
        }
    }
}

void main(void)
{
    usart2_init();

    while (1)
    {
        processed_button = 0;
        uint8_t data_available = ring_buffer_pop(&rx_buffer, &processed_button);

        if (data_available)
        {
            // if read "1", then go to Sleep mode
            if (processed_button == '1')
            {
                process_sleep_mode();
            }
        }
    }
}