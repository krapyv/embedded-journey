#include "led_blink.h"
#include <stdbool.h>

void SystemInit(void)
{
    // empty for now - required by the startup .s
}

void delay(volatile uint32_t count)
{
    // setting a load value (the starting value the counter resets to when it hits zero)
    SYST_RVR = 31999; // 2 ms at 16 MHz

    // clearing the current countdown value
    SYST_CVR = 0;

    // enable the counter (bit 0), set TICKINT to 0 (bit 1) to avoid interrupts and set CLKSOURCE to 1 (bit 2) since we are using processor clock in SYST_CSR
    SYST_CSR |= (1 << 2) | (0 << 1) | (1 << 0); // OR keeps all existing bits and sets the bit 0 to 1, bit 1 to 0 and bit 2 to 1

    // now we need to loop through the values SYST_CVR and count
    for (uint32_t i = 0; i < count; i++)
    {
        while (
            !(SYST_CSR & (1 << 16)))
        {
            // do nothing - hardware is counting down
        }
    }
}

int main()
{
    // first of all, we need to turn the clock enable
    // we already have the macro that gives use the value at the clock address
    // according to the reference manual the bit 0 is the bit that enablea/disables clock for the port A
    // so we need to flip the bit 0 to value 1

    RCC_AHB1ENR |= 1; // the formula is RCC_AHB1ENR |= 1 << n, but n = 0 (because we are changing the first bit

    // next we want to set a direction (GPIOA_MODER)
    // every pin has 2 bits that represent its GPIO_MODER register
    // the first bit can represent the mode 00 - input (reset state) and 01 - the General Input output mode (the one we want)
    // the second bit: 10 - alternate function mode and 11 - analog mode

    // we need to use 01

    // pin PA5 has bits 10 and 11 out from bits 0-31

    // first of all, we need to clear the existing bits 10-11 in the GPIOA_MODER

    // we need to use AND with tilda (NOT, ~) to reset the bits 10-11 but keep all others as they are
    // we are using 0x3, because it is 11 in binary (2^1 + 2^0)
    // we leftshifting the 0x3 by n * 2 => 0x3 << 10
    GPIOA_MODER &= ~(0x3 << 10); // the bits 10-11 are 0 in binary

    // then we need to set the bits 10-11 to desired values: 01
    // we are using 0x1 because it is 01 in binary (2^0)
    // we are using OR to keep all existing bits and set the desired ones
    GPIOA_MODER |= 0x1 << 10;

    while (true)
    {
        // next we need to set the value HIGH to the GPIOA_ODR of the pin PA5
        // since we are using external LED, not the PC13 one, the value 1 is HIGH (3.3V) and 0 is LOW (0V)

        // we want to keep all existing bits and just set the 5th bit to 1
        GPIOA_ODR |= 1 << 5; // GPIOA_ODR |= 1 << n where n = number of pin of the

        // to make the LED blink we need to let it stay ON for some time, then turn it off and then repeat (forever loop)
        // because we are setting the clock enabled and the direction only once, they should not be inside the while(true) loop

        delay(250);

        // then we are turning the led off
        // we basically want to clear the fifth bit (set to 0) and keep all other bits
        // so we are using AND with NOT (NOT (~) gives use binary with all bits as 1 except for the one we want to clear)
        // AND gives use 1 only when two inputs are 1, otherwise 0
        // so we keep all our ones and zeros and clear the fifth bit (set to 0)
        GPIOA_ODR &= ~(1 << 5);

        delay(250);
    }
}