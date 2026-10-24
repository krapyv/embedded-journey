#include <string.h>
#include <stdbool.h>
#include "queue.h"

bool queue_init(Queue_t *queue, void *storage, size_t item_size, uint16_t capacity)
{
    // (capacity & (capacity - 1)) != 0 - check if it is not the power of two
    if (queue == NULL || storage == NULL || item_size == 0 || capacity < 2 || (capacity & (capacity - 1)) != 0)
    {
        return false;
    }

    queue->buffer = storage;
    queue->item_size = item_size;
    queue->mask = capacity - 1;
    queue->head = 0;
    queue->tail = 0;

    return true;
}

bool queue_push(Queue_t *queue, const void *item)
{
    uint16_t head = queue->head;
    uint16_t next = (head + 1) & queue->mask;

    if (next == queue->tail)
    {
        return false; // full
    }

    memcpy(&queue->buffer[head * queue->item_size], item, queue->item_size);

    // data must be written before head moves
    __asm volatile("" ::: "memory"); // compiler barrier

    queue->head = next;

    return true;
}

bool queue_pop(Queue_t *queue, void *item)
{
    uint16_t tail = queue->tail;

    if (tail == queue->head)
    {
        return false; // empty
    }

    memcpy(item, &queue->buffer[tail * queue->item_size], queue->item_size);

    // data must be read before tail moves
    __asm volatile("" ::: "memory");

    queue->tail = (tail + 1) & queue->mask;

    return true;
}

bool queue_is_empty(const Queue_t *queue)
{
    return queue->head == queue->tail;
}

bool queue_is_full(const Queue_t *queue)
{
    return ((queue->head + 1) & queue->mask) == queue->tail;
}