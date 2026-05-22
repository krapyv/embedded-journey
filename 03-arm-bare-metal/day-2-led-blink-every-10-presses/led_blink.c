#include <stdbool.h>
#include "led_blink.h"
#include "button_debouncer.h"

volatile uint32_t ms_passed = 0;

void SystemInit(void)
{
    // empty for now - required by the startup .s
}

// the interrupt service routine (ISR). Tracks how many ms passed
// accepts no arguments, so we nee to create a global variable that keeps the number of miliseconds
void SysTick_Handler(void)
{
    ms_passed++;
}

void delay_ms_init()
{
    SYST_RVR = 15999;

    SYST_CVR = 0;

    // flipping 3 bits: 2 - CLKSOURSE to 1 (processor clock), 1 - TICKINT to 1 (to allow interrups), 0 - ENABLE to 1 (to enale the counter)
    SYST_CSR |= (1 << 2) | (1 << 1) | (1 << 0); // or just SYST_CSR |= 0x07 (0x07 is 0b000111 = 2^2 + 2^1 + 2^0)
}

int main()
{
    ButtonDebouncer_t button_debouncer;
    button_debouncer_init(&button_debouncer, 20);

    RCC_AHB1ENR |= (1 << 0) | (1 << 1);

    GPIOA_MODER &= ~(0x3 << 10); // the bits 10-11 are 0 in binary
    GPIOB_MODER &= ~(0x3);

    GPIOA_MODER |= 0x1 << 10;

    uint32_t last_debounce_call = 0;

    delay_ms_init();

    uint32_t press_count = 0;

    while (true)
    {
        static uint8_t last_button_state = 0;

        if (ms_passed - last_debounce_call >= 100)
        {
            uint8_t raw_input = !((GPIOB_IDR >> 0) & 1);
            uint8_t button_pressed = button_debouncer_update(&button_debouncer, raw_input, 10);

            if (button_pressed && !last_button_state)
            {
                press_count++;
                last_button_state = 1;
            }
            else if (!button_pressed)
            {
                last_button_state = 0;
            }

            if (press_count >= 10)
            {
                GPIOA_ODR ^= (1 << 5);
                press_count = 0;
            }

            last_debounce_call = ms_passed;
        }
    }
}