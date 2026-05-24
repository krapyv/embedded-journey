#include <stdio.h>
#include "ring_buffer.h"

int main(void)
{
    printf("=== Starting Rign Buffer Desktop Testbench ===\n\n");

// Setup a tiny 4-byte buffer (Max capacity will be 3 bytes due to sacrifice slot)
#define TEST_SIZE 4
    RingBuffer_t rx_buffer;
    uint8_t storage[TEST_SIZE];

    // Test the initialization guard with invalid size first
    if (!ring_buffer_init(&rx_buffer, storage, 7))
    {
        printf("[PASS] Initialization safely rejected invalid size 7!\n");
    }

    // Initialize correctly with a power of two
    if (ring_buffer_init(&rx_buffer, storage, TEST_SIZE))
    {
        printf("[PASS] Initialization succeeded for size %d.\n\n", TEST_SIZE);
    }

    // Test filling to the exact capacity limit
    printf("--- Testing Full Capacity Boundary ---\n");
    uint8_t test_data[] = {'A', 'B', 'C', 'D'};

    for (int i = 0; i < 4; i++)
    {
        if (ring_buffer_push(&rx_buffer, test_data[i]))
        {
            printf("Pushed '%c' | Head offset: %d, Tail offset: %d\n", test_data[i], rx_buffer.head, rx_buffer.tail);
        }
        else
        {
            printf("Rejected '%c' | Buffer is FULL! (Head: %d, Tail: %d)\n", test_data[i], rx_buffer.head, rx_buffer.tail);
        }
    }

    // Pop data to clear space and verify contents
    printf("\n--- Testing Pop / Emptying ---\n");
    uint8_t popped_byte;
    while (ring_buffer_pop(&rx_buffer, &popped_byte))
    {
        printf("Popped: '%c' | Head offset: %d, Tail offset: %d\n", popped_byte, rx_buffer.head, rx_buffer.tail);
    }

    if (ring_buffer_is_empty(&rx_buffer))
    {
        printf("Buffer is now completely empty.\n\n");
    }

    // Test the wrapped state
    printf("--- Testing Wrapped Chase State ---\n");
    // Push 2 bytes to step pointers forward
    ring_buffer_push(&rx_buffer, 'X');
    ring_buffer_push(&rx_buffer, 'Y');

    // Pop them immediately to move tail away from 0
    ring_buffer_pop(&rx_buffer, &popped_byte);
    ring_buffer_pop(&rx_buffer, &popped_byte);

    printf("Pointers moved up. Current Head: %d, Tail: %d\n", rx_buffer.head, rx_buffer.tail);

    // Flood it to trigger the wrap-around
    printf("Pushing characters to trigger bitmask rollover:\n");
    uint8_t flood_data[] = {'1', '2', '3'};
    for (int i = 0; i < 3; i++)
    {
        if (ring_buffer_push(&rx_buffer, flood_data[i]))
        {
            printf("Pushed '%c' | Head rolled over to offset: %d\n", flood_data[i], rx_buffer.head);
        }
        else
        {
            printf("Rejected '%c' | Full limit reached!\n", flood_data[i]);
        }
    }

    printf("\n=== Testbench Execution Complete ===\n");

    return 0;
}