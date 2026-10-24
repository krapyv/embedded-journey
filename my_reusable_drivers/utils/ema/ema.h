#ifndef EMA_H
#define EMA_H

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int32_t acc;  // filtered value * 2^(K+FRAC)
    uint8_t k;    // alpha = 2^-k
    uint8_t frac; // fractional bits carried in acc
    bool primed;

} EMA_t;

int32_t EMA_Update(EMA_t *ema, int32_t x);
void EMA_Reset(EMA_t *ema);
bool EMA_Init(EMA_t *ema, uint8_t k, uint8_t frac);

#endif // EMA_H