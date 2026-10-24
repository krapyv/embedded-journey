#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

#define MOTOR_TIMER_CLOCK 16000000U
#define MOTOR_PWM_FREQ 25000U // 25 kHz - silent, 640 steps of about 0.16%
#define MOTOR_PWM_STEPS (MOTOR_TIMER_CLOCK / MOTOR_PWM_FREQ)

void Motor_Init(void);

// u is in [-1, +1]
// sign = direction, magnitude = duty, 0 = brake
uint32_t Motor_Duty_Direction(float u);

#endif // MOTOR_H