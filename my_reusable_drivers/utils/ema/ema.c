#include <stddef.h>
#include "ema.h"

bool EMA_Init(EMA_t *ema, uint8_t k, uint8_t frac)
{
    if (ema == NULL || k < 1 || ((k + frac) > 30))
    {
        return false;
    }

    ema->k = k;
    ema->frac = frac;
    EMA_Reset(ema);

    return true;
}

void EMA_Reset(EMA_t *ema)
{
    ema->acc = 0;
    ema->primed = false;
}

// Input must satisfy |x| < 2^(31 - k - frac)
int32_t EMA_Update(EMA_t *ema, int32_t x)
{
    if (!ema->primed)
    {
        // start with output == x, stored in scaled form
        ema->acc = x * (1 << (ema->k + ema->frac));
        ema->primed = true;
    }
    else
    {
        // multiply instead of << : left-shifting a negative int is Undefined Behavior
        ema->acc += x * (1 << ema->frac) - (ema->acc >> ema->k);
    }

    return ema->acc;
}