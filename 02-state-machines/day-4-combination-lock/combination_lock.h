#ifndef COMBINATION_LOCK_H
#define COMBINATION_LOCK_H

#include <stdint.h>

typedef enum
{
    LOCKED,
    DIGIT_1,
    DIGIT_2,
    DIGIT_3,
    UNLOCKED,
    LOCK_TIMEOUT
} CombinationLockState_t;

typedef struct
{
    CombinationLockState_t current_state;
    uint8_t combination[4]; // 4 digits
    uint8_t entered[4];
    uint8_t entered_digits;
    uint8_t wrong_attempts;
    uint32_t timeout_timer_ms;
    uint32_t unlock_timer_ms;
} CombinationLock_t;

// functions headers
void combination_lock_init(CombinationLock_t *lock, uint8_t combination[4]);
void combination_lock_update(CombinationLock_t *lock, uint8_t last_entered, uint32_t interval_ms);

#endif