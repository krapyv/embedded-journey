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

#endif // APP_CONFIG_H