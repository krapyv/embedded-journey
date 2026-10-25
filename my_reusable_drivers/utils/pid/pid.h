#ifndef PID_H
#define PID_H

#include <stdbool.h>

typedef struct
{
    float Kp, Ki, Kd;
    float dt; // CONTROL_TICK_S ((float)CONTROL_TICK_MS / 1000.0f)
    float target;
    float integral;
    float prevMeasured;
    float outMin, outMax; // for clamping
    bool hasPrev;         // false until first sample after init/reset
} PID_t;

bool PID_Init(PID_t *pid, float target, float Kp, float Ki, float Kd, float dt, float outMin, float outMax);
float PID_Update(PID_t *pid, float measured);

#endif // PID_H