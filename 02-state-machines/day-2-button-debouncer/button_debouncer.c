#include <stdint.h>
#include <stdbool.h>
#include "button_debouncer.h"
#include "simulated_gpio.h"

#define DEFAULT_DEBOUNCE_DELAY 15;

void button_debouncer_init(ButtonDebouncer_t *debouncer, uint32_t delay_ms)
{
    debouncer->current_state = RELEASED;
    debouncer->debounce_delay_ms = delay_ms;
    debouncer->timer_ms = 0;
}

// button pressing sequence:
// RELEASED -> PRESSING -> PRESSED -> RELEASING
int button_debouncer_update(ButtonDebouncer_t *debouncer, bool raw_input, uint32_t interval_ms)
{
    bool output;
    switch (debouncer->current_state)
    {
    case RELEASED:
        break;

    case PRESSING:
        break;

    case PRESSED:
        break;

    case RELEASING:
        break;
    }

    return output;
}
