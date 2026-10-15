#ifndef APP_CONFIG_H
#define APP_CONFIG_H

typedef struct
{
    float Kp, Ki, Kd;
    float integral;
    float prevMeasured;
    float outMin, outMax; // for clamping
} PID_t;

#endif