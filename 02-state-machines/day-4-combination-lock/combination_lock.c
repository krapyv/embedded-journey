#include "combination_lock.h"
#include <stdint.h>
#include <stdio.h>

// to avoid magical numbers, we are using variables
#define COMBINATION_DIGIT_COUNT 4
#define MAX_WRONG_ATTEMPTS 3

#define TIMEOUT_MS 10000
#define UNLOCK_MS 5000

#define DIGIT_1_IDX 0
#define DIGIT_2_IDX 1
#define DIGIT_3_IDX 2
#define DIGIT_4_IDX 3

void combination_lock_print_status(CombinationLock_t *lock)
{
    printf("\n=== COMBINATION LOCK ===\n");

    switch (lock->current_state)
    {
    case LOCKED:
        printf("State: LOCKED\n");
        printf("Entered: ");
        for (int i = 0; i < lock->entered_digits; i++)
        {
            printf("%d ", lock->entered[i]);
        }
        for (int i = lock->entered_digits; i < COMBINATION_DIGIT_COUNT; i++)
        {
            printf("_ ");
        }
        printf("\nAttempts remaining: %d\n", MAX_WRONG_ATTEMPTS - lock->wrong_attempts);
        break;

    case DIGIT_1:
    case DIGIT_2:
    case DIGIT_3:
        printf("State: ENTERING CODE\n");
        printf("Entered: ");
        for (int i = 0; i < lock->entered_digits; i++)
        {
            printf("%d ", lock->entered[i]);
        }
        for (int i = lock->entered_digits; i < COMBINATION_DIGIT_COUNT; i++)
        {
            printf("_ ");
        }
        printf("\n");
        break;

    case UNLOCKED:
        printf("State: *** UNLOCKED ***\n");
        printf("Access granted. Will relock in %d seconds.\n", (UNLOCK_MS - lock->unlock_timer_ms) / 1000);
        break;

    case LOCK_TIMEOUT:
        printf("State: LOCKED OUT\n");
        printf("Too many wrong attemps. Wait %d seconds.\n", (TIMEOUT_MS - lock->timeout_timer_ms) / 1000);
        break;
    }

    printf("======================\n");
}

// static functions that are used only inside this file
static void combination_lock_handle_reset_entered(uint8_t entered[4], uint8_t *counter)
{
    for (int i = 0; i < COMBINATION_DIGIT_COUNT; i++)
    {
        entered[i] = 0;
    }

    // dereferencing the pointer to access its value, not an address
    *counter = 0;
}

// helper function to handle wrong attempts
static void combination_lock_handle_wrong_attempt(CombinationLock_t *lock)
{
    lock->wrong_attempts++;

    // reset the entered array and counter of the entered digits (in every case we are starting again from the LOCKED state or having LOCK_TIMEOUT state and after that LOCKED one)
    combination_lock_handle_reset_entered(lock->entered, &lock->entered_digits);

    if (lock->wrong_attempts >= MAX_WRONG_ATTEMPTS)
    {
        lock->current_state = LOCK_TIMEOUT;
        lock->timeout_timer_ms = 0;
    }
    else
    {
        lock->current_state = LOCKED;
    }
}

void combination_lock_init(CombinationLock_t *lock, uint8_t combination[4])
{
    lock->current_state = LOCKED;

    // we could use copy element by element or memcpy from the library string.h
    for (int i = 0; i < COMBINATION_DIGIT_COUNT; i++)
    {
        lock->combination[i] = combination[i];
    }

    combination_lock_handle_reset_entered(lock->entered, &lock->entered_digits);

    lock->wrong_attempts = 0;
    lock->timeout_timer_ms = 0;
    lock->unlock_timer_ms = 0;
}

void combination_lock_update(CombinationLock_t *lock, uint8_t last_entered, uint32_t interval_ms)
{
    // we are accepting only 1-9 digits (0 is the flag that means "no digit")
    if (last_entered > 9)
        return;

    // during LOCK_TIMEOUT and UNLOCKED, we need to ignore all new input digits
    if (lock->current_state != LOCK_TIMEOUT && lock->current_state != UNLOCKED)
    {
        lock->entered[lock->entered_digits] = last_entered;
        lock->entered_digits++;
    }

    // switch case state machine mechanism
    // we are going to check for the wrong digit inside the switch statement
    switch (lock->current_state)
    {
    case LOCKED:
        if (last_entered != lock->combination[DIGIT_1_IDX])
        {
            combination_lock_handle_wrong_attempt(lock);
            break;
        }

        lock->current_state = DIGIT_1;
        break;

    case DIGIT_1:
        if (last_entered != lock->combination[DIGIT_2_IDX])
        {
            combination_lock_handle_wrong_attempt(lock);
            break;
        }

        lock->current_state = DIGIT_2;
        break;

    case DIGIT_2:
        if (last_entered != lock->combination[DIGIT_3_IDX])
        {
            combination_lock_handle_wrong_attempt(lock);
            break;
        }

        lock->current_state = DIGIT_3;
        break;

    case DIGIT_3:
        if (last_entered != lock->combination[DIGIT_4_IDX])
        {
            combination_lock_handle_wrong_attempt(lock);
            break;
        }

        // we need to reset the wrong_attempts counter if we did not fall to combination_lock_handle_wrong_attempt
        lock->wrong_attempts = 0;
        combination_lock_handle_reset_entered(lock->entered, &lock->entered_digits);
        lock->current_state = UNLOCKED;
        break;

    case UNLOCKED:
        lock->unlock_timer_ms += interval_ms;
        if (lock->unlock_timer_ms >= UNLOCK_MS)
        {
            lock->current_state = LOCKED;
            lock->wrong_attempts = 0;
            lock->unlock_timer_ms = 0;
        }
        break;

    case LOCK_TIMEOUT:
        lock->timeout_timer_ms += interval_ms;
        if (lock->timeout_timer_ms >= TIMEOUT_MS)
        {
            lock->current_state = LOCKED;

            // without it the lock stays in LOCK_TIMEOUT forever
            lock->wrong_attempts = 0;
            lock->timeout_timer_ms = 0;
        }
        break;
    }
}