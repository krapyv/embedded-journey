#ifndef LED_H
#define LED_H

#include <stdint.h>

// generic LED handle
typedef struct
{
    volatile uint32_t *moder_reg;   // pointer to the GPIO MODER register
    volatile uint32_t *odr_reg;     // pointer to the GPIO ODR register
    volatile uint32_t *rcc_clk_reg; // pointer to the clock register
    uint8_t pin;                    // pin number (from 0 to 15)
    uint8_t rcc_bit;                // bit position in RCC_AHB1ENR to enable the clock
} LED_HandleTypeDef;

void led_init(LED_HandleTypeDef *hled);
void led_on(LED_HandleTypeDef *hled);
void led_off(LED_HandleTypeDef *hled);

#endif