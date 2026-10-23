#include "core_cm4.h"
#include "stm32f411.h"
#include "app_config.h"

void TIM2_init(void)
{
    // a timer needs to produce an interrupt once in 10 ms

    // enable RCC TIM2 clock
    RCC->APB1ENR |= (1UL << 0U);

    // ensure the CMS bits (6:5) in CR1 are 00 - edge-aligned mode, so the counter counts up or down depending on the direction bit (DIR)
    // 11 = 0x3
    TIM2->CR1 &= ~(0x3UL << 5U);

    // set the upcounting direction: bit 4 DIR to 0
    TIM2->CR1 &= ~(1UL << 4U);

    // ensure the UDIS (bit 1) is 0, so UEV is enabled
    TIM2->CR1 &= ~(1UL << 1U);

    // set the URS (bit 2) - Update request source - to 1
    // so only counter overflow/underflow generates an update interrupt
    TIM2->CR1 |= (1UL << 2U);

    // set UIE (bit 0) of DIER to 1 to enable Update interrupts
    TIM2->DIER |= (1UL << 0U);

    // set auto-reload register
    // HSI oscillator has by default 16MHz frequency
    // 1 ms = 16 000 ticks. then 10 ms = 10 * 16 000 = 160 000
    // ARR = 159 999 - since TIM2 has a 32-bit ADD register, it is ok
    TIM2->ARR = 159999;

    // make sure PSC is set to 0
    TIM2->PSC = 0;

    // make sure CNT is set to 0 (due to the possible re-init or a bootloader handoff)
    TIM2->CNT = 0;

    // make sure there is no stale pending Update interrupt
    TIM2->SR &= ~(1UL << 0U);

    // TIM2 Handler has the position 28 in the vector table
    // the position 28 is the ISER0 bit 28
    // set the TIM2 a priority of 8
    NVIC->IPR[28] = ((uint8_t)TIM2_INTERRUPT_PRIORITY_LEVEL << 4U);

    NVIC->ISER[0] = (1UL << 28U);

    // enable counter
    TIM2->CR1 |= (1UL << 0U);
}

void main()
{
}