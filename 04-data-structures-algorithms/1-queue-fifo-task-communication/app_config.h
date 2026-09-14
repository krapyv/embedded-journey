#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdbool.h>

#define MCP2515_INTERRUPT_PRIORITY_LEVEL 4U
#define BASEPRI_MASK_INTERRUPT_PRIORITY_LEVEL 0U

typedef enum
{
    Payload_BMP280 = 0,
    Payload_MCP2515
} Payload_ProducerTypes_t;

typedef struct
{
    uint8_t SIDH;
    uint8_t SIDL;
    uint8_t EID8;
    uint8_t EID0;
    uint8_t DLC;
    uint8_t SIDH;
    uint8_t data[8];
} CAN_Frame_t;

typedef struct
{
    int32_t temp_value;
    uint32_t press_value;
} BMP280_Reading_t;

union Payload_Messages
{
    CAN_Frame_t mcp2515_frame;
    BMP280_Reading_t bmp280_measurements;
};

typedef struct
{
    union Payload_Messages payload;
    Payload_ProducerTypes_t producer_type
} Payload_t;

typedef struct
{
    Payload_t *buffer;
    volatile uint8_t head;
    volatile uint8_t tail;
    uint8_t max_size;
} Queue_t;

bool queue_init(Queue_t *queue, Payload_t *storage, uint8_t size);
bool queue_push(Queue_t *queue, Payload_t message);
bool queue_pop(Queue_t *queue, Payload_t *message);
bool queue_is_empty(Queue_t *queue);
bool queue_is_full(Queue_t *queue);

#endif // APP_CONFIG_H