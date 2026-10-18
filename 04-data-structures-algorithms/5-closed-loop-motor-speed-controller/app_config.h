#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "stdint.h"

/* UART2 CONFIGURATION */
#define UART_MODE_TX_ONLY 0U
#define UART_MODE_RX_ONLY 1U
#define UART_MODE_TX_RX 2U

// select active mode for the project
#define TARGET_UART_MODE UART_MODE_TX_ONLY

/* FILTER MODES */
#define UNFILTERED_BASELINE 0U
#define EMA_IN_LOOP_FILTER_MODE 1U
#define MEDIAN_EMA_IN_LOOP_FILTER_MODE 2U
#define EMA_OUTSIDE_LOOP_FILTER_MODE 3U

// select active filter mode for the project
#define FILTER_MODE UNFILTERED_BASELINE

typedef struct
{
    float Kp, Ki, Kd;
    float integral;
    float prevMeasured;
    float outMin, outMax; // for clamping
} PID_t;

typedef struct
{
    int32_t raw;
    int32_t filtered;
    int32_t setpoint;
    int32_t PID_output;
    uint32_t tick;
} Logger_t;

typedef struct
{
    int32_t acc; // holds filtered value * 2^(K+FRAC)
    bool primed;
} EMA_t;

#endif