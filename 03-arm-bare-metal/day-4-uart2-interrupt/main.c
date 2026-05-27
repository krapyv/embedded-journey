#include "uart.h"
#include "ring_buffer.h"
#include "led.h"

extern RingBuffer_t rx_buffer; // access the global storage from uart.c
extern volatile uint32_t ms_passed;

int main(void)
{
    LED_HandleTypeDef led_pa5;
    led_pa5.moder_reg = (volatile uint32_t *)(0x40020000 + 0x00);
    led_pa5.odr_reg = (volatile uint32_t *)(0x40020000 + 0x14);
    led_pa5.rcc_clk_reg = (volatile uint32_t *)(0x40023800 + 0x30);
    led_pa5.pin = 5;
    led_pa5.rcc_bit = 0;

    usart2_init();
    led_init(&led_pa5);
    timer_init();

    uint8_t proccessed_char;
    uint32_t last_transmission = 0;

    while (1)
    {
        // critical section - thread-safe-data extraction

        // disable USART2 interrupts temporarily at the NVIC level
        // it prevents the ISR from breaking in while we evaluate the buffer pointers!

        // Because the NVIC_ISERx and NVIC_ICERx registers are Write-1-to-Set and Write-1-to-Clear hardware latches,
        // using the bitwise OR assignment operator (|=) is completely redundant,
        // slower, and can technically introduce a tiny timing window you don't want.
        NVIC_ICER1 = (1 << 6);

        proccessed_char = 0;
        // we are safely checking and extracting data from our ring buffer
        uint8_t data_available = ring_buffer_pop(&rx_buffer, &proccessed_char);

        // re-enable USART2 interrupts immediately
        NVIC_ISER1 = (1 << 6);

        // process received data
        if (data_available)
        {
            // simple command parser
            // if read "1", turn on an LED on pin PA5
            if (proccessed_char == '1')
            {
                led_on(&led_pa5);
            }
            else if (proccessed_char == '0')
            {
                led_off(&led_pa5);
            }

            proccessed_char = 0;
        }

        // asynchronous non-blocking TX timing
        if (ms_passed - last_transmission >= 500)
        {
            usart2_write_string("System Status: OK\r\n");
            last_transmission = ms_passed;
        }
    }
}