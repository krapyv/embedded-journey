#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    uint8_t *buffer;        // raw storage, owned by the caller
    size_t item_size;       // bytes per element
    uint16_t mask;          // capacity - 1
    volatile uint16_t head; // written only by the producer
    volatile uint16_t tail; // written only by the consumer
} Queue_t;

bool queue_init(Queue_t *queue, void *storage, size_t item_size, uint16_t capacity);
bool queue_push(Queue_t *queue, const void *item);
bool queue_pop(Queue_t *queue, void *item);
bool queue_is_empty(const Queue_t *queue);
bool queue_is_full(const Queue_t *queue);

#endif // QUEUE_H