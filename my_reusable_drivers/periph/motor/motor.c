#include <math.h>
#include "../../../my_reusable_drivers/core/stm32f411.h"
#include "motor.h"

#define TIM4_ARR (MOTOR_PWM_STEPS - 1U)

static float Motor_Clamp(float u)
{
    if (isnan(u))
    {
        return 0.0f;
    }
    if (u > 1.0f)
    {
        return 1.0f;
    }
    if (u < -1.0f)
    {
        return -1.0f;
    }

    return u;
}

// The PID output is clamped to [0,1], so reverse only exists for open-loop tests
uint32_t Motor_Duty_Direction(float u)
{
    // Clamp

    u = Motor_Clamp(u);

    // Direction

    if (u > 0)
    {
        // v > 0 - forward
        // IN1 = 1 (H), IN2 = 0 (L)
        GPIOB->BSRR = ((1 << 29U) | (1 << 12U));
    }
    else if (u < 0)
    {
        // v < 0 - reverse
        // IN1 = 0 (L), IN2 = 1 (H)
        GPIOB->BSRR = ((1 << 28U) | (1 << 13U));
    }
    else
    {
        // u = 0 - brake
        // IN1 = 1 (H), IN2 = 1 (H)
        GPIOB->BSRR = (0x3 << 12U);
    }

    float abs_u = fabsf(u);

    // Magnitude
    // CCR[2] = (ARR + 1) * u

    uint32_t CCR3 = (TIM4_ARR + 1) * abs_u + 0.5f;

    TIM4->CCR[2] = CCR3;

    return CCR3;
}

void Motor_Disable(void)
{
    GPIOB->BSRR = (1 << 30U); // turning the motor OFF
}

void Motor_Init(void)
{
    // PB12, PB13, PB14

    // enable GPIOB clock
    RCC->AHB1ENR |= (1 << 1U);

    // set the PB12 and PB13 pins high
    // 0b11 = 2^1 + 2^0 = 2 + 1 = 3 = 0x3
    // set the PB14 (STBY) pin low
    // 0b1 = 0x1
    GPIOB->BSRR = ((0x1 << 30U) | (0x3 << 12U));

    // set the pins as outputs
    // MODER as 01 - General purpose output mode
    // 0b01 = 0x1
    // for PB12 - bits 25:24, PB13 - bits 27:26, PB14 - 29:28

    // clear the bits
    // 0b11 = 0x3
    GPIOB->MODER &= ~((0x3 << 28U) | (0x3 << 26U) | (0x3 << 24U));

    // set the bits
    GPIOB->MODER |= ((0x1 << 28U) | (0x1 << 26U) | (0x1 << 24U));

    // make sure the pins are set to Output push-pull output type
    // OTYPER = 0

    // clear the bits 14:12
    // 0b111 = 0x7
    GPIOB->OTYPER &= ~(0x7 << 12U);

    // configure PB8 for the TIM4

    // set its MODER to the Alternate Function (10)
    // 0b10 = 0x2

    // clear the bits 17:16 (the MODER gives pin two bits)
    // 0b11 = 0x3
    GPIOB->MODER &= ~(0x3 << 16U);

    // set the bits
    GPIOB->MODER |= (0x2 << 16U);

    // set PB8 to AF2 (TIM4 uses AF2)
    // AF2 = 0010 = 0x2

    // 8 > 7, then use GPIOB->AFRH

    // clear the bits 3:0
    // 0x1111 = 2^3 + 2^2 + 2^1 + 2^0 = 8 + 4 + 2 + 1 = 0xF
    GPIOB->AFRH &= ~(0xF << 0U);

    // set the bits
    GPIOB->AFRH |= (0x2 << 0U);

    // ----- TIM4 Configuration -----

    // enable TIM4 clock
    RCC->APB1ENR |= (1 << 2U);

    // reset the CNT
    TIM4->CNT = 0;

    // make sure the PSC is 0
    TIM4->PSC = 0;

    // set the auto-reload (ARR) value
    TIM4->ARR = TIM4_ARR;

    // clear the ARPE (bit 7) in CR1
    TIM4->CR1 &= ~(1 << 7U);

    // set the initial value of CCR3
    TIM4->CCR[2] = 0;

    // configure CCMR2

    // set CC3S (bits 1:0) to 00 (output)

    // clear the bits
    // 11 = 2^1 + 2^0 = 2 + 1 = 0x3
    TIM4->CCMR2 &= ~(0x3 << 0U);

    // set OC3PE (bit 3) to 1 - enable preload
    TIM4->CCMR2 |= (1 << 3U);

    // set 0C3M (bits 6:4) to PWM Mode 1 - 110
    // 0b110 = 2^2 + 2^1 = 4 + 2 = 0x6

    // clear the bits
    // 0b111 = 0x7
    TIM4->CCMR2 &= ~(0x7 << 4U);

    // set the bits
    TIM4->CCMR2 |= (0x6 << 4U);

    // configure CCER

    // set CC3E (bit 8) to 1 to enable the output for channel 3
    TIM4->CCER |= (1 << 8U);

    // clear CC3P (bit 9) and CC3NP (bit 11) to set the polarity
    // 101 = 2^2 + 2^0 = 5 = 0x5
    TIM4->CCER &= ~(0x5 << 9U);

    // force the update
    TIM4->EGR = (1 << 0U);

    // enable the counter
    TIM4->CR1 |= (1 << 0U);

    // raise the STBY to high to turn the driver ON
    GPIOB->BSRR = (1 << 14U);
}