#include <stdio.h>
#include <stdint.h>
#include "combination_lock.h"

void simple_correct_combination_test(CombinationLock_t *lock)
{
    // correct sequence
    uint8_t sequence[] = {1, 2, 3, 4};

    for (int i = 0; i < 4; i++)
    {
        printf("\n>>> Pressing digit: %d\n", sequence[i]);
        combination_lock_update(lock, sequence[i], 10);
        combination_lock_print_status(lock);
    }
}

// helper function to simulate 5/10 seconds passing
void simulate_time_passing(CombinationLock_t *lock, uint32_t ms)
{
    uint32_t elapsed = 0;
    int count = 0;
    while (elapsed < ms)
    {
        // we are not passing any digit
        combination_lock_update(lock, 0, 10);
        elapsed += 10;
        count++;
    }
}

void simple_incorrect_combination_test(CombinationLock_t *lock)
{
    uint8_t sequence[] = {1, 3, 3, 3};

    for (int i = 0; i < 4; i++)
    {
        printf("\n>>> Pressing digit: %d\n", sequence[i]);
        combination_lock_update(lock, sequence[i], 10);
        combination_lock_print_status(lock);
    }

    simulate_time_passing(lock, 10000); // we are waiting for 10 seconds
    combination_lock_print_status(lock);
}

int main(void)
{
    CombinationLock_t lock;
    uint8_t combination[4] = {1, 2, 3, 4};

    combination_lock_init(&lock, combination);
    combination_lock_print_status(&lock);

    simple_correct_combination_test(&lock);

    // simple_incorrect_combination_test(&lock);

    return 0;
}