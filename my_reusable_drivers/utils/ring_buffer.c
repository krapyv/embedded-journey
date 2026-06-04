#include <stdbool.h>
#include "ring_buffer.h"

bool ring_buffer_init(RingBuffer_t *rb, uint8_t *storage_array, uint32_t size)
{
    // the non-negotiable condition: size must be an exact power of two (4, 8, 16 ...)
    // to check if the size is power of two: (size & (size - 1)) == 0 -> the number is a power of two
    bool is_power_of_two = (size & (size - 1)) == 0;
    if (size <= 0 || !is_power_of_two)
    {
        return false;
    }

    // at the beginning, both head and tail are pointing to 0, because they are empty
    rb->buffer = storage_array;
    rb->head = 0;
    rb->tail = 0;
    rb->max_size = size;

    return true;
}
bool ring_buffer_push(RingBuffer_t *rb, uint8_t byte)
{
    // check if the buffer is full (no slots to push into)
    if (ring_buffer_is_full(rb))
    {
        return false;
    }

    // since the head points to the first empty slot in the array, the new byte are inserted into this position and the head moves to the next empty slot
    rb->buffer[rb->head] = byte; // rb->buffer is a pointer (point to the first element of the array)

    // next we should check whether the head are wrapping around to the beginning
    // if head is at the last index, it is going to have the position of 0
    // if it is not yet at the verge of the array, it will have its next index
    // ((head + 1) & (mask)) = NEW_POSITION where mask = SIZE - 1
    rb->head = ((rb->head + 1) & (rb->max_size - 1));

    return true;
}

bool ring_buffer_pop(RingBuffer_t *rb, uint8_t *byte)
{
    // check if the buffer is empty (no elements to pop)
    if (ring_buffer_is_empty(rb))
    {
        return false;
    }
    // since the tail is pointing to the first unread element in the array, we can just get this element by accessing the element with index of tail
    *byte = rb->buffer[rb->tail];

    // we need to check whether the tail is wrapping around to the beginning
    // if it is not yet at the verge of the array, it will have its next index; otherwise - index 0
    rb->tail = ((rb->tail + 1) & (rb->max_size - 1));

    return true;
}

bool ring_buffer_is_empty(RingBuffer_t *rb)
{
    // the empty condition: head and tail are pointing to the same index
    return rb->head == rb->tail;
}

bool ring_buffer_is_full(RingBuffer_t *rb)
{
    // the fully condition:
    // ((head + 1) & mask) == tail;

    // mask = max_size - 1;
    return ((rb->head + 1) & (rb->max_size - 1)) == rb->tail;
}
