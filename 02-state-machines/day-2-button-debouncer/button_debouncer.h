#ifndef BUTTON_DEBOUNCER_H
#define BUTTON_DEBOUNCER_H

#include <stdint.h>

typedef enum
{
    RELEASED,
    PRESSING,
    PRESSED,
    RELEASING
} ButtonDebouncerState_t;

typedef struct
{
    ButtonDebouncerState_t current_state;
    uint32_t debounce_delay_ms;
    uint32_t timer_ms;
    bool prev_raw_input; // to detect edge (0 -> 1 or 1 -> 0)
    bool debounced_state;
} ButtonDebouncer_t;

void button_debouncer_init(ButtonDebouncer_t *debouncer, uint32_t delay_ms); // we do not need to pass anything except for the delay_ms: first state - released, debounced delay - default 15ms, timer - 0, debounced state - 0

int button_debouncer_update(ButtonDebouncer_t *debouncer, bool raw_input, uint32_t interval_ms); // we need to know how much time passed since last call to increment timer_ms

#endif