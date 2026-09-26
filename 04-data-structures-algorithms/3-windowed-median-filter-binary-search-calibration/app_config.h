#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/* UART2 CONFIGURATION */
#define UART_MODE_TX_ONLY 0U
#define UART_MODE_RX_ONLY 1U
#define UART_MODE_TX_RX 2U

// select active mode for the project
#define TARGET_UART_MODE UART_MODE_TX_ONLY

typedef enum
{
    TYPE_INT32,
    TYPE_UINT32
} DataType;

typedef struct
{
    uint32_t pressure;
    int32_t altitude;
} LUT_Pressure_Altitude_t;

#endif