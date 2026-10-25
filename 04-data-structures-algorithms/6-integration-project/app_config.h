#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

/* FILTER MODES */
#define UNFILTERED_BASELINE 0U
#define EMA_IN_LOOP_FILTER_MODE 1U
#define MEDIAN_EMA_IN_LOOP_FILTER_MODE 2U
#define EMA_OUTSIDE_LOOP_FILTER_MODE 3U

// select active filter mode for the project
#define FILTER_MODE EMA_IN_LOOP_FILTER_MODE

#define MCP2515_INTERRUPT_PRIORITY_LEVEL 4U
#define TIM2_INTERRUPT_PRIORITY_LEVEL 1U

#define COUNTS_PER_REV 3840U // measured over 10 revolutions, matching 3840 counts/rev
#define TIMER_CLOCK 16000000U

#define CONTROL_TICK_MS 15U                               // 15 ms
#define CONTROL_TICK_S ((float)CONTROL_TICK_MS / 1000.0f) // 15 ms

#define KP 0.03f
#define KI 0.5f
#define KD 0.0f

typedef enum
{
    Payload_Timer = 0,
    Payload_MCP2515
} Payload_ProducerTypes_t;

typedef struct
{
    uint8_t SIDH;
    uint8_t SIDL;
    uint8_t EID8;
    uint8_t EID0;
    uint8_t DLC;
    uint8_t data[8];
} CAN_Frame_t;

union Payload_Messages
{
    CAN_Frame_t mcp2515_frame;
    uint32_t tick_count;
};

typedef struct
{
    union Payload_Messages payload;
    Payload_ProducerTypes_t producer_type;
} Payload_t;

typedef struct
{
    int32_t raw;
    int32_t filtered;
    int32_t setpoint;
    int32_t PID_output;
    uint32_t tick;
} Logger_t;

/* UART2 CONFIGURATION */
#define UART_MODE_TX_ONLY 0U
#define UART_MODE_RX_ONLY 1U
#define UART_MODE_TX_RX 2U

// select active mode for the project
#define TARGET_UART_MODE UART_MODE_TX_ONLY
#endif // APP_CONFIG_H