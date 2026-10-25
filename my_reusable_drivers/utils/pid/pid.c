#include <stdbool.h>
#include <math.h>
#include <stddef.h>
#include "pid.h"

bool PID_Init(PID_t *pid, float target, float Kp, float Ki, float Kd, float dt, float outMin, float outMax)
{
    if (pid == NULL || dt <= 0.0f || outMin >= outMax)
    {
        return false;
    }

    pid->target = target;
    pid->dt = dt;
    pid->integral = 0.0f;
    pid->prevMeasured = 0.0f;
    pid->outMin = outMin;
    pid->outMax = outMax;

    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;

    pid->hasPrev = false;

    return true;
}

// TODO: no-guard comparison on the real motor
// TODO: encoder-loss or stall detection (if the encoder cable falls off, the loop reads 0 RPM and drives u to 1.0)
float PID_Update(PID_t *pid, float measured)
{
    if (!isfinite(measured))
    {
        // safe fall back
        // returns false if the argument is infinite or NAN
        return pid->outMin;
    }

    float error = pid->target - measured;

    float P = pid->Kp * error;

    float I = pid->Ki * pid->integral;

    float derivative = 0.0;
    if (pid->hasPrev)
    {
        derivative = (measured - pid->prevMeasured) / pid->dt;
    }
    pid->prevMeasured = measured;
    pid->hasPrev = true;

    float D = -pid->Kd * derivative;

    float output = P + I + D;
    bool pushingFurther = (output > pid->outMax && error > 0) || (output < pid->outMin && error < 0);

    if (!pushingFurther)
    {
        pid->integral += error * pid->dt;
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
