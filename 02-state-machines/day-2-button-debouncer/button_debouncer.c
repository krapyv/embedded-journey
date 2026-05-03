#include <stdint.h>
#include <stdbool.h>
#include "button_debouncer.h"

void button_debouncer_init(ButtonDebouncer_t *debouncer, uint32_t delay_ms)
{
    debouncer->state = RELEASED;
    debouncer->debounce_delay_ms = delay_ms;
    debouncer->timer_ms = 0;
}

// button pressing sequence:
// RELEASED -> PRESSING -> PRESSED -> RELEASING
bool button_debouncer_update(ButtonDebouncer_t *debouncer, bool raw_input, uint32_t interval_ms)
{
    // the timer changes only in the pressing and releasing states
    switch (debouncer->state)
    {
    case RELEASED:
        if (raw_input == 1)
        {
            debouncer->timer_ms = 0;
            debouncer->state = PRESSING;
            return 0;
        }
        return 0;

    case PRESSING:
        debouncer->timer_ms += interval_ms;
        // if during the pressing state the button debounces, we need to fall back to the released state
        if (raw_input == 0)
        {
            debouncer->timer_ms = 0;
            debouncer->state = RELEASED;
            return 0;
        }

        // if the "stabilization time" passed and the raw_input always was 1 (because if not, the previous if would have worked and threw us back to the RELEASED state), we could advance to the pressed state
        if (debouncer->timer_ms >= debouncer->debounce_delay_ms && raw_input == 1)
        {
            // the timer is used to count time in the current state
            // during the transition to PRESSED or RELEASED, we are exiting the debouncing states
            // debouncer->timer_ms = 0;
            debouncer->state = PRESSED;
            return 1;
        }
        return 0;

    case PRESSED:
        // when the first raw_input as a zero comes up, we need to advance to the releasing state
        if (raw_input == 0)
        {
            debouncer->timer_ms = 0;
            debouncer->state = RELEASING;

            return 0; // immediate release detection, we are not waiting for debounce
        }
        return 1;

    case RELEASING:
        debouncer->timer_ms += interval_ms;

        if (raw_input == 1)
        {
            debouncer->timer_ms = 0;
            debouncer->state = PRESSING;

            return 0;
        }

        if (debouncer->timer_ms >= debouncer->debounce_delay_ms && raw_input == 0)
        {
            debouncer->state = RELEASED;
            return 0;
        }

        return 0;
    }
}
