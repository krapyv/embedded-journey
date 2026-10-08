#include <stdbool.h>
#include "i2c.h"
#include "core_cm4.h"
#include "stm32f411.h"
#include "systick.h"
#include "app_config.h"

I2C_HandleTypeDef hi2c;
PID_t pid;

#define TIME_CONSTANT 150U                                // 150 ms
#define CONTROL_TICK_MS 15U                               // 15 ms
#define CONTROL_TICK_S ((float)CONTROL_TICK_MS / 1000.0f) // 15 ms

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

    speed += ((float)CONTROL_TICK_MS / (float)TIME_CONSTANT) * (K * u - speed);

    u = PID_Update(&pid, speed);

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
    PID_Init();
    TIM2_Init();

    while (1)
    {
    }
}