#include <stdbool.h>
#include "led_blink.h"
#include "button_debouncer.h"

void SystemInit(void)
{
    // empty for now - required by the startup .s
}

void delay_ms(uint32_t ms)
{
    for (volatile uint32_t i = 0; i < ms * 8000; i++)
        ;
}

int main()
{
    ButtonDebouncer_t button_debouncer;
    button_debouncer_init(&button_debouncer, 20);

    RCC_AHB1ENR |= (1 << 0) | (1 << 1);

    GPIOA_MODER &= ~(0x3 << 10); // the bits 10-11 are 0 in binary
    GPIOB_MODER &= ~(0x3);

    GPIOA_MODER |= 0x1 << 10;

    while (true)
    {

        uint8_t raw_input = !((GPIOB_IDR >> 0) & 1);
        uint8_t button_pressed = button_debouncer_update(&button_debouncer, raw_input, 10);

        if (button_pressed)
        {

            GPIOA_ODR |= (1 << 5); // GPIOA_ODR |= 1 << n where n = number of pin of the
        }
        else
        {

            GPIOA_ODR &= ~(1 << 5);
        }

        delay_ms(10);
    }
}