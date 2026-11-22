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
#define TARGET_UART_MODE UART_MODE_TX_ISR

#define LOGGER_QUEUE_LENGTH 6U
#define LOGGER_QUEUE_ITEM_SIZE sizeof(LogEvent_t)

#define SENSOR_TASK_PERIOD_MS 500U

#define UART_TX_BUFFER_SIZE 512U

// priorities
#define PRIORITY_SENSOR_TASK 4U
#define PRIORITY_LOGGER_TASK 1U

#define STACK_TASK_DEPTH 256U

#endif // APP_CONFIG_H