#include "led.h"
#include <stdbool.h>

void led_init(LED_HandleTypeDef *hled)
{
    // first of all, enable a clock
    *hled->rcc_clk_reg |= (1 << hled->rcc_bit);

    // next set according bits in MODER to 01 (output)
    // MODER operates on two bits, so 1 bit = 2 * pin and 2 bit = (2 * pin) + 1

    // clear the bits first
    *hled->moder_reg &= ~((0x3 << (2 * hled->pin))); // 0x3 = 0b0011 -> after invert: 0b1100

    // now set to 01
    // 0x1 = 0b001

    *hled->moder_reg |= (0x1 << (2 * hled->pin));
}

void led_on(LED_HandleTypeDef *hled)
{
    // next we need to set the value HIGH to the GPIO_ODR of the pin
    // since we are using external LED, not the PC13 one, the value 1 is HIGH (3.3V) and 0 is LOW (0V)

    *hled->odr_reg |= (1 << hled->pin);
}

void led_off(LED_HandleTypeDef *hled)
{
    *hled->odr_reg &= ~(1 << hled->pin); // we want to keep all existing bits and clear the bit PIN, so AND with NOT (~)
}
