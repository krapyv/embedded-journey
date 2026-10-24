#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

// Quadrature encoder on TIM1 (CH1 = PA8, CH2 = PA9), x4 decoding, 16-bit counter
void Encoder_Init(void);

// Signed counts moved since the previous call (sign == direction)
// Stateful: call from exactly one context (the TIM2 control tick)
// | delta | < 32768 per call
int16_t Encoder_ReadDelta(void);

#endif // ENCODER_H