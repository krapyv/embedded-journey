#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

/* UART2 CONFIGURATION */
#define UART_MODE_TX_ONLY 0U
#define UART_MODE_RX_ONLY 1U
#define UART_MODE_TX_RX 2U
#define UART_MODE_TX_RX_POLLING 3U
#define UART_MODE_TX_ISR 4U

// select active mode for the project
#define TARGET_UART_MODE UART_MODE_TX_RX_POLLING

typedef enum
{
    EVT_CAN_TX_OK = 0,
    EVT_CAN_TX_ERROR,
    EVT_SENSOR_READ_OK,
    EVT_SENSOR_READ_FAIL,
    EVT_QUEUE_FULL,
    EVT_BUTTON_STATUS,
} LogEvent_ID_e;

typedef struct
{
    uint32_t arg0;
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
    uint32_t tick;
    uint8_t event_id;
} LogEvent_t;

#define LOGGER_QUEUE_LENGTH 6U
#define LOGGER_QUEUE_ITEM_SIZE sizeof(LogEvent_t)

#define SENSOR_TASK_PERIOD_MS 500U

#endif // APP_CONFIG_H