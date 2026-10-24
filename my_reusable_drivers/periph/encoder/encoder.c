#include <stdint.h>
#include "encoder.h"
#include "../../../my_reusable_drivers/core/stm32f411.h"

static uint16_t prev_count;

void Encoder_Init(void)
{
    // TIM1: CH1 - PA8 and CH2 - PA9
    // enable GPIOA clock
    RCC->AHB1ENR |= (1 << 0UL);

    // enable TIM1 clock
    RCC->APB2ENR |= (1 << 0UL);

    // ----- CONFIGURE GPIOA PA8 and PA9 -----
    // change MODER to Alternate Function
    // 10: Alternate function mode -> 10 = 0x2

    // clear the bits
    // PA8 takes bits 17:16 and PA9 takes bits 19:18
    // 0b1111 = 2^3 + 2^2 + 2^1 + 2^0 = 8 + 4 + 2 + 1 = 0xF
    GPIOA->MODER &= ~(0xF << 16U);

    // set the bits
    // PA8 - 0b10, PA9 - 0b10
    // 0b1010 = 2^3 + 2^1 = 8 + 2 = 10 = 0xA
    GPIOA->MODER |= (0xA << 16U);

    // make sure GPIOA_OTYPER is push-pull (0)
    // 11 = 0x3
    GPIOA->OTYPER &= ~(0x3 << 8U);

    // make sure there is no pull-up, pull-down
    // PA8 takes bits 17:16, PA9 takes bits 19:18
    // 0b1111 = 0xF
    // clear the bits
    GPIOA->PUPDR &= ~(0xF << 16U);

    // set the alternate function values for PA8 and PA9
    // TIM1 uses AF1 (0001)

    // PA8 and PA9 are affected by GPIO_AFRH (ports 8 and 9)
    // PA8 takes bits 3:0, PA9 takes bits 7:4

    // clear the bits
    // 0b1111 = 0xF
    GPIOA->AFRH &= ~((0xF << 4U) | (0xF << 0U));

    // set bits
    // 0b0001 = 0x1
    GPIOA->AFRH |= ((0x1 << 4U) | (0x1 << 0U));

    // ----- CONFIGURE THE TIM1 -----

    // set the SMS (bits 2:0) in TIM1_SMCR
    // Encoder mode 3 - counter counts up/down on both TI1FP1 and T2FP2 edges
    // 011 = 0x3

    // clear the bits
    // 0b111 = 2^2 + 2^1 + 2^0 = 4 + 2 + 1 = 7 = 0x7
    TIM1->SMCR &= ~(0x7 << 0U);

    // set the bits
    TIM1->SMCR |= (0x3 << 0U);

    // set the CC1S to 01 (bits 1:0) and CC2S to 01 (bits 9:8) in TIM1_CCMR1

    // clear the bits
    // 0b11 = 0x3
    TIM1->CCMR1 &= ~((0x3 << 8U) | (0x3 << 0U));

    // 0b01 = 0x1
    TIM1->CCMR1 |= ((0x1 << 8U) | (0x1 << 0U));

    // set the TIM1_ARR to 0xFFFF
    TIM1->ARR = 0xFFFF;

    // make sure the prescaler is 0 (f_ck_psc / (PSC[15:0] + 1)) => (f_ck_psc / 1)
    TIM1->PSC = 0;

    // reset the CNT
    TIM1->CNT = 0;

    // set the polarity in TIM1_CCER
    // CC1P as 00 (bit 1 + bit 3 of CC1NP)
    TIM1->CCER &= ~((0x1 << 3U) | (0x1 << 1U));

    // CC2P as 00 (bit 5 + bit 7 of CC2NP)
    TIM1->CCER &= ~((0x1 << 7U) | (0x1 << 5U));

    // Delta start
    prev_count = (uint16_t)TIM1->CNT;

    // enable counter
    TIM1->CR1 |= (1 << 0U);
}

int16_t Encoder_ReadDelta(void)
{
    uint16_t now = (uint16_t)TIM1->CNT;
    // 2^16, the wrap is free
    uint16_t diff = (uint16_t)(now - prev_count);
    prev_count = now;
    return (int16_t)diff;
}
