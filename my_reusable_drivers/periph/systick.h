#ifndef SYSTICK_H
#define SYSTICK_H

#include <stdint.h>

// function headers

// init function
void SysTick_Init(uint32_t frequency_in_hz);

// getter function to ensure encapsulation
uint32_t SysTick_GetTick(void);

// helper function to replace all software-defined loops
void SysTick_Delay_ms(uint32_t ms);

#endif