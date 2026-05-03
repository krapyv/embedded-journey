#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "button_debouncer.h"

#define DEFAULT_DEBOUNCE_DELAY 20

uint32_t current_time_ms = 0;

// the function to simulate a timer interrupt (call this evert 5ms in test)
void timer_interrupt(void)
{
    current_time_ms += 5;
}

// bounce pattern generator
// returns true (1) or false (0) based on a predefined bouncing sequence
bool get_bouncing_button_signal(uint32_t time_ms)
{
    // simulate a button press with bounce at time 100ms
    if (time_ms >= 100 && time_ms < 200)
    {
        // press event with bounce: 1, 0, 1, 0, 1, 1, 1, 1
        if (time_ms < 105)
            return true;
        if (time_ms < 110)
            return false; // bounce
        if (time_ms < 115)
            return true;
        if (time_ms < 120)
            return false;
        if (time_ms < 125)
            return true;

        return true; // after 125ms, stable pressed
    }

    // simulate a button release with bounce at time 300ms
    if (time_ms >= 300 && time_ms < 400)
    {
        // press event with bounce: 0, 1, 0, 1, 0, 0, 0, 0
        if (time_ms < 305)
            return false; // first break
        if (time_ms < 310)
            return true;
        if (time_ms < 315)
            return false;
        if (time_ms < 320)
            return true;
        if (time_ms < 325)
            return false;

        return false; // after 325ms, stable released
    }

    return false; // not pressed
}

bool get_bouncing_button_signal_simpler(uint32_t time_ms)
{
    if (time_ms >= 100 && time_ms < 300)
    {
        // 10ms of 1, 5ms of 0, 10ms of 1, then stable pressed

        uint32_t t = time_ms - 100;

        if (t < 10)
            return true; // first press
        if (t < 15)
            return false;
        if (t < 25)
            return true;
        if (t < 30)
            return false;

        return true; // stable pressed
    }

    return false; // not pressed
}

void print_state(uint32_t time_ms, bool raw, bool debounced)
{
    // A static variable inside a function keeps its value between invocations.
    static uint32_t last_print_ms = 0;

    printf("%4dms: raw=%d, debounce=%d\n", time_ms, raw, debounced);
    last_print_ms = time_ms;
}

int main(void)
{
    ButtonDebouncer_t button;

    button_debouncer_init(&button, DEFAULT_DEBOUNCE_DELAY);
    printf("--- Button Debouncer ---\n");
    printf("Debounce delay = %d\n", DEFAULT_DEBOUNCE_DELAY);

    while (current_time_ms < 500)
    {
        bool raw_input = get_bouncing_button_signal(current_time_ms); // we are getting "raw" input

        bool debounced = button_debouncer_update(&button, raw_input, 5);

        print_state(current_time_ms, raw_input, debounced);

        timer_interrupt();
    }
    return 0;
}
