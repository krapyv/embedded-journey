#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

/* UART2 CONFIGURATION */

// select active mode for the project
#define TARGET_UART_MODE UART_MODE_TX_ISR

// logger queue
#define LOGGER_QUEUE_LENGTH 6U
#define LOGGER_QUEUE_ITEM_SIZE sizeof(LogEvent_t)

// sensor queue
#define SENSOR_QUEUE_LENGTH 1U
#define SENSOR_QUEUE_ITEM_SIZE sizeof(SensorQueue_t)

#define SENSOR_TASK_PERIOD_MS 500U

#define UART_TX_BUFFER_SIZE 512U

// priorities
#define PRIORITY_SENSOR_TASK 4U
#define PRIORITY_LOGGER_TASK 1U

#define STACK_TASK_DEPTH 256U

/* MCP2515 CONFIGURATION */
#define MCP_INTERRUPT_MODE MCP_INTERRUPTS_DISABLED

#endif // APP_CONFIG_H