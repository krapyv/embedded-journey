#include "led_blink.h"
#include <stdbool.h>

void SystemInit(void)
{
    // empty for now - required by the startup .s
}

int main()
{
    // first of all, we need to turn the clock enable
    // we already have the macro that gives use the value at the clock address
    // according to the reference manual the bit 0 is the bit that enablea/disables clock for the port A
    // so we need to flip the bit 0 to value 1

    // we also need to enable the clock of port B (according to the reference manual, it is a bit 1)
    // the formula is RCC_AHB1ENR |= 1 << n
    RCC_AHB1ENR |= (1 << 0) | (1 << 1);

    //
    // next we want to set a direction (GPIOA_MODER and GPIOB_MODER)
    // every pin has 2 bits that represent its GPIO_MODER register
    // the first bit can represent the mode 00 - input (reset state what we want to the port B) and 01 - the General Input output mode (the one we want for port A)
    // the second bit: 10 - alternate function mode and 11 - analog mode

    // we need to use 00 and 01

    // pin PA5 has bits 10 and 11 out from bits 0-31
    // pin PB0 has bits 0 and 1

    // first of all, we need to clear the existing bits 0-1 and 10-11 in the GPIOA_MODER

    // we need to use AND with tilda (NOT, ~) to reset the desired bits but keep all others as they are

    // for both pins we are using 0x3, because it is 11 in binary (2^1 + 2^0)
    // for PA5 we leftshifting the 0x3 by n * 2 => 0x3 << 10
    // for PB0 we leftshifting the 0x3 by 0 => 0x3
    GPIOA_MODER &= ~(0x3 << 10); // the bits 10-11 are 0 in binary
    GPIOB_MODER &= ~(0x3);

    // then we need to set the bits 10-11 of port A to desired values: 01
    // we are using 0x1 because it is 01 in binary (2^0)
    // we are using OR to keep all existing bits and set the desired ones
    GPIOA_MODER |= 0x1 << 10;

    // for pin 0 of the port B (bits 0-1) we do not need to do anything
    // because it is already what we need: 00 for input mode

    // now we need to read the value of bit 0 of the GPIOB_IDR waiting for the 0 which is HIGH in our active-low circuit

    // we need to have infinite loop to always look for the GPIOB_IDR values
    while (true)
    {
        // basically we need to extract the bit 0 from GPIOB_IDR to have the desired value
        // to do that, we need to rightshift the value of GPIOB_IDR by n bits, so our desired bit will be the first bit
        // then we need to clear all other bits except the one we want to extract

        // e.g. 0b00011001, n = 3
        // 0b00011001 >> 3 = 0b00000011
        // then we mask it by 1 << 0 (because the bit we want it already at the position 0)
        // 0b00000011 & 1 = 0b00000011 & 0b00000001 = 0b00000001 = 1

        uint8_t button_pressed = !((GPIOB_IDR >> 0) & (1 >> 0));

        // if value is 0x1, the button should be released
        // if value is 0x0, the button is pressed
        // in active-low circuits, 1 is LOW (0V), 0 is HIGH (3.3V)

        if (button_pressed)
        {
            // we need to set the value HIGH to the GPIOA_ODR of the pin PA5
            // since we are using external LED, not the PC13 one, the value 1 is HIGH (3.3V) and 0 is LOW (0V)

            // we want to keep all existing bits and just set the 5th bit to 1
            GPIOA_ODR |= (1 << 5); // GPIOA_ODR |= 1 << n where n = number of pin of the
        }
        else
        {
            // we are turning the led off
            // we basically want to clear the fifth bit (set to 0) and keep all other bits
            // so we are using AND with NOT (NOT (~) gives use binary with all bits as 1 except for the one we want to clear)
            // AND gives use 1 only when two inputs are 1, otherwise 0
            // so we keep all our ones and zeros and clear the fifth bit (set to 0)
            GPIOA_ODR &= ~(1 << 5);
        }
    }
}