# Combination lock

A finite state machine implementation of a 4-digit combination lock with error handling and timeout. Built in C with no external libraries.

## Overview

A 6-state finite state machine (LOCKED, DIGIT_1, DIGIT_2, DIGIT_3, UNLOCKED, LOCK_TIMEOUT) that uses a configurable combination, tracks wrong attempts, and implements a 10-second timeout after 3 failures. The project includes an auto-lock mechanism that relocks 5 seconds after successful entry.

## State Diagram

Initial State: LOCKED

LOCKED - [digit 1 input]:
if correct: DIGIT_1
if incorrect: LOCKED
if wrong_attempts == 3: LOCK_TIMEOUT

DIGIT_1 - [digit 2 input]:
if correct: DIGIT_2
if incorrect: LOCKED
if wrong_attempts == 3: LOCK_TIMEOUT

DIGIT_2 - [digit 3 input]:
if correct: DIGIT_3
if incorrect: LOCKED
if wrong_attempts == 3: LOCK_TIMEOUT

DIGIT_3 - [digit 4 input]:
if correct: UNLOCKED
if incorrect: LOCKED
if wrong_attempts == 3: LOCK_TIMEOUT

UNLOCKED:
5s timeout, then LOCKED

LOCK_TIMEOUT:
10s timeout, then LOCKED

## Features

- **6-state finite state machine**: LOCKED -> DIGIT_1 -> DIGIT_2 -> DIGIT_3 -> UNLOCKED -> LOCK_TIMEOUT
- **Wrong attempt tracking**: Increments on incorrect digits, resets on correct unlock
- **Timeout mechanism**: 10-second lockout after 3 wrong attempts
- **Auto-relock**: Automatically relocks 5 seconds after successful unlock
- **Configurable combination**: Set any 4-digit code at initialization
- **Non-blocking timers**: Counter-based timing with configurable interval

## Requirements

- GCC or any other C compiler
- Standard C libraries

## Building

```bash
gcc combination_lock.c test_combination_lock.c -o test_combination_lock
./test_combination_lock
```

## Usage

```c

void simple_correct_combination_test(CombinationLock_t *lock)
{
    uint8_t sequence[] = {1, 2, 3, 4};

    for (int i = 0; i < 4; i++)
    {
        printf("\n>>> Pressing digit: %d\n", sequence[i]);
        combination_lock_update(lock, sequence[i], 10);
        combination_lock_print_status(lock);
    }
}

CombinationLock_t lock;
uint8_t combination[4] = {1, 2, 3, 4};

combination_lock_init(&lock, combination);
combination_lock_print_status(&lock);
   
simple_correct_combination_test(&lock);
```

## Output

```text

=== COMBINATION LOCK ===
State: LOCKED
Entered: _ _ _ _ 
Attempts remaining: 3
======================

>>> Pressing digit: 1

=== COMBINATION LOCK ===
State: ENTERING CODE
Entered: 1 _ _ _ 
======================

>>> Pressing digit: 2

=== COMBINATION LOCK ===
State: ENTERING CODE
Entered: 1 2 _ _ 
======================

>>> Pressing digit: 3

=== COMBINATION LOCK ===
State: ENTERING CODE
Entered: 1 2 3 _ 
======================

>>> Pressing digit: 4

=== COMBINATION LOCK ===
State: *** UNLOCKED ***
Access granted. Will relock in 5 seconds.
======================

```

## Project Structure

```text
├── combination_lock.h      # API for the lock logic
├── combination_lock.c      # Switch-case state machine implementation 
├── test_combination_lock.c # Testing application
└── README.md          # This .md file
```

## API Reference

### Combination Lock Operations

| Function | Description |
| ---------| ----------- |
| combination_lock_init() | Initialize the combination lock state. |
| combination_lock_update() | Update state machine and process timeouts. |

### Combination Lock Helper Functions

| Function | Description |
| -------- | ----------- |
| combination_lock_print_status | Prints current lock state to display |
| combination_lock_handle_reset_entered | Resets entered combination to initial state |
| combination_lock_handle_wrong_attempt | Handles a wrong digit input |

### State Machine States

| State | Description |
| ----- | ----------- |
| LOCKED | Initial state |
| DIGIT_1 | The first digit correct |
| DIGIT_2 | The second digit correct |
| DIGIT_3 | The third digit correct |
| UNLOCKED | The entered combination is correct |
| LOCK_TIMEOUT | A state with 10s timeout after 3 wrong attempts |

## Author

Dmytro Krapyvianskyi

## License

MIT
