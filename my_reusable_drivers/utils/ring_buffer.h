#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t *buffer;
    uint32_t head;
    uint32_t tail;
    uint32_t max_size;
} RingBuffer_t;

bool ring_buffer_init(RingBuffer_t *rb, uint8_t *storage_array, uint32_t size);
bool ring_buffer_push(RingBuffer_t *rb, uint8_t byte);
bool ring_buffer_pop(RingBuffer_t *rb, uint8_t *byte);
bool ring_buffer_is_empty(RingBuffer_t *rb);
bool ring_buffer_is_full(RingBuffer_t *rb);

#endif
