#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

// compilation modes
#define NORMAL_MODE 0U
#define PRIORITY_INVERSION_TEST 1U

#define COMPILATION_MODE NORMAL_MODE

/* UART2 CONFIGURATION */

// select active mode for the project
#define TARGET_UART_MODE UART_MODE_TX_ISR

// logger queue
#define LOGGER_QUEUE_LENGTH 6U
#define LOGGER_QUEUE_ITEM_SIZE sizeof(LogEvent_t)

// sensor queue
#define SENSOR_QUEUE_LENGTH 1U
#define SENSOR_QUEUE_ITEM_SIZE sizeof(SensorReading_t)

#define SENSOR_TASK_PERIOD_MS 500U

#define UART_TX_BUFFER_SIZE 512U

#define PRIORITY_LOGGER_TASK 1U

#if (COMPILATION_MODE == NORMAL_MODE)
// priorities
#define PRIORITY_SENSOR_TASK 4U
#define PRIORITY_CAN_TX_TASK 3U
#define PRIORITY_STATUS_TASK 2U
#endif

#if (COMPILATION_MODE == PRIORITY_INVERSION_TEST)
#define PRIORITY_H_TASK 4U
#define PRIORITY_M_TASK 3U
#define PRIORITY_L_TASK 2U
#endif

// must be >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
#define BUTTON_IRQ_PRIORITY 5U

#define STACK_TASK_DEPTH 256U

#define BINARY_LOCK 0U
#define MUTEX_LOCK 1U

#define PI_LOCK_TYPE MUTEX_LOCK

/* MCP2515 CONFIGURATION */
#define MCP_INTERRUPT_MODE MCP_INTERRUPTS_DISABLED

/* CNF values (CAN bit-timing layer) derived for 500 kbit/s */

// BRP[5:0] = 000000, SJW[1:0] = 00 = 1 Tq
// 0x00
#define CAN_CNF1 0x00
// BTLMODE = 1 (PS2 length set by PHSEG2 (CNF3), not auto-derived)
// SAM = 0 - single sample at sample point
// PHSEG1[2:0] = 010 = PS1 = 3 Tq
// PRSEG[2:0] = 001 = PropSeg = 2 Tq
// CNF2 = BITMODE | SAM | PHSEG1[2:0] | PRSEG[2:0]
// 0x91
#define CAN_CNF2 0x91
// PHSEG2[2:0] = 001 = 2 Tq
// 0x01
#define CAN_CNF3 0x01

// Put together, that's SyncSeg(1) + PropSeg(2) + PS1(3) + PS2(2) = 8 Tq per bit,
// with BRP=0 giving Tq = 2×Tosc= 250ns (at Fosc=8MHz)
// — so 8 Tq × 250ns = 2µs per bit = 500 kbit/s, and the sample point lands after
// SyncSeg+PropSeg+PS1 = 6 of the 8 Tq, i.e. 75% into the bit.

#endif // APP_CONFIG_H