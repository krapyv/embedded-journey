#ifndef SAMPLING_H
#define SAMPLING_H

#include "stm32f411.h"

#define USART2_CLK_MANAGED_EXTERNALLY

/* Plain integer tokens for the preprocessor */
#define HARDWARE_PORT_A 0U
#define HARDWARE_PORT_B 1U

#define TARGET_SAMP_PORT HARDWARE_PORT_A

#define SAMP_PORT GPIOA

typedef struct
{
    GPIO_RegDef_t *Port;
} SAMP_HandleTypeDef;

#endif