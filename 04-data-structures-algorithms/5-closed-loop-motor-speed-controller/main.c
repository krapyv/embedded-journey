#include <stdbool.h>
#include "stm32f411.h"
#include "core_cm4.h"
#include "systick.h"
#include "i2c.h"
#include "app_config.h"

I2C_HandleTypeDef hi2c;

PID_t pid;

#define TIME_CONSTANT 150U                                // 150 ms
#define CONTROL_TICK_MS 15U                               // 15 ms
#define CONTROL_TICK_S ((float)CONTROL_TICK_MS / 1000.0f) // 15 ms
#define COUNTS_PER_REV 3840U                              // measured over 10 revolutions, matching 16 poles * 4 edges * 120

#define KP 0.1f
#define KI 0.5f // tuned on τ = 150 ms placeholder plant, retune on real motor
#define KD 0.0f

volatile uint32_t TIM2_counter;
uint32_t SYSTICK_start;
volatile uint32_t elapsed_ms;
volatile float speed;
volatile float K = 160;
volatile float u = 0;
volatile float target = 80;
volatile uint16_t prev_count;

float PID_Update(PID_t *pid, float measured_speed)
{
    float error = target - measured_speed;

    float P = pid->Kp * error;

    float I = pid->Ki * pid->integral;

    float derivative = (measured_speed - pid->prevMeasured) / CONTROL_TICK_S;
    float D = -pid->Kd * derivative;

    pid->prevMeasured = measured_speed;

    float output = P + I + D;
    bool pushingFurther = (output > pid->outMax && error > 0) || (output < pid->outMin && error < 0);

    if (!pushingFurther)
    {
        pid->integral += error * CONTROL_TICK_S;
    }

    // clamp the output
    if (output < pid->outMin)
    {
        output = pid->outMin;
    }
    if (output > pid->outMax)
    {
        output = pid->outMax;
    }

    return output;
}

void PID_Init()
{
    pid.integral = 0.0f;
    pid.prevMeasured = 0.0f;
    pid.outMin = 0.0f;
    pid.outMax = 1.0f;

    pid.Kp = KP;
    pid.Ki = KI;
    pid.Kd = KD;
}

void TIM2_IRQHandler(void)
{
    // clear an update interrupt flag, so the ISR won't re-enter forever
    TIM2->SR &= ~(1 << 0U);

    TIM2_counter++;
    uint16_t current_cnt = TIM1->CNT;

    int16_t delta = (uint16_t)(current_cnt - prev_count);

    // speed += ((float)CONTROL_TICK_MS / (float)TIME_CONSTANT) * (K * u - speed);
    speed = (float)delta / (COUNTS_PER_REV * CONTROL_TICK_S) * 60.0f;

    u = PID_Update(&pid, speed);

    prev_count = current_cnt;

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
    prev_count = TIM1->CNT;

    // enable counter
    TIM2->CR1 |= (1 << 0U);
}

void TIM1_Init(void)
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

    // enable counter
    TIM1->CR1 |= (1 << 0U);
}

void main(void)
{
    SCB->CPACR = (0xFU << 20U);

    __DSB();
    __ISB();

    SysTick_Init(SYSTICK_FREQUENCY_16MHZ);
    PID_Init();
    TIM1_Init();
    TIM2_Init();

    while (1)
    {
    }
}