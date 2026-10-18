#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "stm32f411.h"
#include "core_cm4.h"
#include "uart.h"
#include "systick.h"
#include "app_config.h"

PID_t pid;

#define TIME_CONSTANT 150U                                // 150 ms
#define CONTROL_TICK_MS 15U                               // 15 ms
#define CONTROL_TICK_S ((float)CONTROL_TICK_MS / 1000.0f) // 15 ms
#define COUNTS_PER_REV 3840U                              // measured over 10 revolutions, matching 3840 counts/rev

#define TIMER_CLOCK 16000000U
#define PWM_FREQ 25000U // 25 kHz - silent, 640 steps of about 0.16%
#define TIM4_ARR ((TIMER_CLOCK / PWM_FREQ) - 1)

#define KP 0.03f
#define KI 0.5f // tuned on τ = 135 ms placeholder plant, retune on real motor
#define KD 0.0f // TODO: Kd and the setpoint-kic on the real-motor

// digital filter

/* the filter's own gain
the update is acc += (x << FRAC) - (acc >> K)
it settles when the two terms cancel:
acc >> K = x << FRAC
acc = x << (FRAC + K)

so in steady state acc is x scaled by 2^(K + FRAC), not just 2^FRAC
the >> K in the update divides by 2^K, so acc has to be 2^K times larger to balance it
*/
#define EMA_K 1 // alpha = 2^(-K) = 0.5
// acc keeps 4 bits below the integer point, so the filter tracks differences down to 1/16 of a count instead of stalling
#define EMA_FRAC 4
#define EMA_SHIFT (EMA_K + EMA_FRAC) // EMA_K + EMA_FRAC = 5, total number of extra bits acc carries

#define EMA_SCALE (1 << EMA_SHIFT)

volatile uint32_t TIM2_counter;
uint32_t SYSTICK_start;
volatile uint32_t elapsed_ms;
volatile float speed;
volatile float K = 160; // measured at 6 V
volatile float u = 0;
volatile float target = 80;
volatile uint16_t prev_count;

volatile bool open_loop_duty = false;

volatile float step_log[100];
volatile uint8_t step_log_index; // 256 samples max
volatile bool step_log_recording = false;

EMA_t ema_filt;
volatile Logger_t logger;

uint32_t last_seen = 0;
bool logger_primed = false;

void EMA_Reset(EMA_t *ema)
{
    ema->acc = 0;
    ema->primed = false;
}

// returns the value scaled by EMA_SCALE
int32_t EMA_Update(EMA_t *ema, int32_t x)
{
    if (!ema->primed)
    {
        /* acc is not the filtered value, it is the filtered value times 2^EMA_SHIFT,
        so to make the filter start at "the output equals x",
        we need to store x in the scaled form */

        /*
        ema->acc = (x << EMA_SHIFT), but if the speed delta x is negative, left-shifting a negative signed number is undefined behavior in C
        */
        ema->acc = x * (1 << EMA_SHIFT);
        ema->primed = true;
    }
    else
    {

        /* ema->acc += (x << EMA_FRAC) - (ema->acc >> EMA_K);

        x << EMA_FRAC - left-shifting a negative signed number is undefined behavior in C.
        The multiplication is well-defined for negative x, and the compiler still generates a shift.

        So ema->acc += (x << EMA_FRAC) - (ema->acc >> EMA_K) => ema->acc += x * (1 << EMA_FRAC) - (ema->acc >> EMA_K)
        */
        ema->acc += x * (1 << EMA_FRAC) - (ema->acc >> EMA_K);
    }

    return ema->acc;
}

// TODO: no-guard comparison on the real motor
// TODO: encoder-loss or stall detection (if the encoder cable falls off, the loop reads 0 RPM and drives u to 1.0)
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

// The PID output is clamped to [0,1], so reverse only exists for open-loop tests
void motor_carrier_duty_direction(void)
{
    // Clamp

    if (u > 1.0f)
    {
        u = 1.0f;
    }
    else if (u < -1.0f)
    {
        u = -1.0f;
    }

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

    uint32_t CCR2 = (TIM4_ARR + 1) * abs_u + 0.5f;

    TIM4->CCR[2] = CCR2;

    logger.PID_output = CCR2;
}

void TIM2_IRQHandler(void)
{
    // clear an update interrupt flag, so the ISR won't re-enter forever
    TIM2->SR &= ~(1 << 0U);

    // TIM2_counter++;
    uint16_t current_cnt = TIM1->CNT;

    int16_t delta = (uint16_t)(current_cnt - prev_count);
    int32_t acc = EMA_Update(&ema_filt, delta);

    (void)acc;
// speed += ((float)CONTROL_TICK_MS / (float)TIME_CONSTANT) * (K * u - speed);
#if (FILTER_MODE == UNFILTERED_BASELINE)
#warning "baseline branch compiled"
    speed = (float)delta / (COUNTS_PER_REV * CONTROL_TICK_S) * 60.0f;
    logger.filtered = delta * EMA_SCALE;
#elif (FILTER_MODE == EMA_IN_LOOP_FILTER_MODE)
    speed = (float)acc / EMA_SCALE / (COUNTS_PER_REV * CONTROL_TICK_S) * 60.0f;
    logger.filtered = acc;
#elif (FILTER_MODE == MEDIAN_EMA_IN_LOOP_FILTER_MODE)
#error "Unimplemented MEDIAN_EMA_IN_LOOP_FILTER_MODE"
#elif (FILTER_MODE == EMA_OUTSIDE_LOOP_FILTER_MODE)
#error "Unimplemented EMA_OUTSIDE_LOOP_FILTER_MODE"
#else
#error "Unknown filter mode"
#endif

    if (step_log_recording)
    {

        if (step_log_index < 100)
        {
            step_log[step_log_index] = speed;
            step_log_index++;
        }
        else
        {
            step_log_recording = false;
        }
    }

    if (!open_loop_duty)
    {
        u = PID_Update(&pid, speed);
    }
    prev_count = current_cnt;

    logger.raw = delta;
    logger.setpoint = (int32_t)target;
    logger.tick++;

    motor_carrier_duty_direction();

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

void motor_carrier_init(void)
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

void logger_snapshot()
{
    if (logger_primed && logger.tick == last_seen)
    {
        return; // discard duplicates
    }
    uint32_t seqlock_start;
    Logger_t local_logger;
    uint32_t tick_after_copy;

    do
    {
        seqlock_start = logger.tick;

        __COMPILER_BARRIER();
        local_logger.raw = logger.raw;
        local_logger.filtered = logger.filtered;
        local_logger.setpoint = logger.setpoint;
        local_logger.PID_output = logger.PID_output;
        local_logger.tick = seqlock_start;
        __COMPILER_BARRIER();

        tick_after_copy = logger.tick;
    } while (tick_after_copy != seqlock_start);

    last_seen = tick_after_copy;

    printf("%ld,%ld,%ld,%ld,%lu\n", local_logger.raw, local_logger.filtered, local_logger.setpoint, local_logger.PID_output, local_logger.tick);

    logger_primed = true;
}

void main(void)
{
    SCB->CPACR = (0xFU << 20U);

    __DSB();
    __ISB();

    SysTick_Init(SYSTICK_FREQUENCY_16MHZ);
    PID_Init();

    // the open-loop legacy code
    // open_loop_duty = true;
    u = 0;

    usart2_init();
    printf("# mode=%u,scale=%d,cpr=%u,tick_ms=%u,ccr_full=%u\n", FILTER_MODE, EMA_SCALE, COUNTS_PER_REV, CONTROL_TICK_MS, (TIM4_ARR + 1));
    TIM1_Init();
    motor_carrier_init();
    TIM2_Init();

    while (1)
    {
        logger_snapshot();
    }
}