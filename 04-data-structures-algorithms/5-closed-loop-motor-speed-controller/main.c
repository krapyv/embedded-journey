#include "i2c.h"
#include "core_cm4.h"
#include "stm32f411.h"
#include "systick.h"

I2C_HandleTypeDef hi2c;

#define TIME_CONSTANT 150U  // 150 ms
#define CONTROL_TICK_MS 15U // 15 ms

volatile uint32_t TIM2_counter;
uint32_t SYSTICK_start;
volatile uint32_t elapsed_ms;
volatile float speed;
float target = 100.0;

void TIM2_IRQHandler(void)
{
    // clear an update interrupt flag, so the ISR won't re-enter forever
    TIM2->SR &= ~(1 << 0U);

    // read the target into a local variable
    float local_target = target;

    speed += (CONTROL_TICK_MS / TIME_CONSTANT) * (local_target - speed);

    TIM2_counter++;
    // NOTE: 15ms testing

    // if (TIM2_counter == 67U)
    // {
    //     elapsed_ms = SysTick_GetTick() - SYSTICK_start;
    // }
}

void TIM2_Init(void)
{
    // TIM2 is a 32-bit timer, clocked at 16 MHz from HSI

    // make sure APB1 prescaler is 1 (AHB clock is not divided)
    // bits 12:10 - 0xx AHB clock not divided
    // 111 = 2^2 + 2^1 + 2^0 = 4 + 2 + 1 = 7 = 0x7
    RCC->CFGR &= ~(0x7 << 10U);

    // enable TIM2 clock
    RCC->APB1ENR |= (1 << 0U);

    // set the prescaler to 0
    TIM2->PSC = 0;

    // set the CNT to 0
    TIM2->CNT = 0;

    // set the ARR (Auto-reload value) to 239 999 (240k ticks = 15ms)
    TIM2->ARR = 239999;

    // make sure ARPE is 0
    TIM2->CR1 &= ~(1 << 7U);

    // clear a possible stale update interrupt flag
    TIM2->SR &= ~(1 << 0U);

    // ------ NVIC config ------
    // set priority to TIM2 (more urgent = smaller numerical value)
    // the core implements only the top 4 bits, so << 4 is mandatory to get it right
    NVIC->IPR[28] = (0x1 << 4U);

    // set priority to SysTick (less urgent = bigger numerical value)
    // also implemented only the top 4 bits, so << 4 is mandatory as well
    // bits 31:28
    // 1111 = 2^3 + 2^2 + 2^1 + 2^0 = 8 + 4 + 2 + 1 = 15 = 0xF
    SCB->SHPR3 &= ~(0xFU << 28U);

    // enable the NVIC for TIM2 (position 28 in the vector table)
    NVIC->ISER[0] = (1 << 28U);

    // set to the priority of 2 (0x2)
    SCB->SHPR3 |= (0x2 << 28U);

    // ------ NVIC config ------

    // enable Update interrupt
    TIM2->DIER |= (1 << 0U);

    SYSTICK_start = SysTick_GetTick();

    // enable counter
    TIM2->CR1 |= (1 << 0U);
}

void main(void)
{
    SCB->CPACR = (0xFU << 20U);

    __DSB();
    __ISB();

    SysTick_Init(SYSTICK_FREQUENCY_16MHZ);
    TIM2_Init();

    while (1)
    {
    }
}