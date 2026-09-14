#include "app_config.h"
#include "i2c.h"
#include "bmp280.h"
#include "mcp2515.h"

I2C_HandleTypeDef hi2c;

volatile uint8_t can_int_flag = 0;
volatile uint8_t can_bus_off = 0;
volatile uint8_t can_intf_stuck = 0;

bool queue_init(Queue_t *queue, Payload_t *storage, uint8_t size)
{
    bool is_power_of_two = (size & (size - 1)) == 0;
    if (size <= 0 || !is_power_of_two)
    {
        return false;
    }

    queue->buffer = storage;
    queue->head = 0;
    queue->tail = 0;
    queue->max_size = size;

    return true;
}

bool queue_push(Queue_t *queue, Payload_t message)
{
    if (queue_is_full(queue))
    {
        return false;
    }

    queue->buffer[queue->head] = message;

    queue->head = ((queue->head + 1) & (queue->max_size - 1));

    return true;
}

bool queue_pop(Queue_t *queue, Payload_t *message)
{
    if (queue_is_empty(queue))
    {
        return false;
    }

    *message = queue->buffer[queue->tail];

    queue->tail = ((queue->tail + 1) & (queue->max_size - 1));

    return true;
}

bool queue_is_empty(Queue_t *queue)
{
    return queue->head == queue->tail;
}

bool queue_is_full(Queue_t *queue)
{
    return ((queue->head + 1) & (queue->max_size - 1)) == queue->tail;
}

void main(void)
{
}