#ifndef FLASH_CONFIG
#define FLASH_CONFIG

#include <stdbool.h>

typedef enum
{
    FLASH_OK = 0,
    FLASH_ERROR = 1
} FLASH_ReturnTypes_t;

typedef enum
{
    FLASH_RDERR = (1UL << 8U),
    FLASH_PGSERR = (1UL << 7U),
    FLASH_PGPERR = (1UL << 6U),
    FLASH_PGAERR = (1UL << 5U),
    FLASH_WRPERR = (1UL << 4U),
} FLASH_ErrorCodes_t;

typedef struct
{
    uint8_t start_byte;
    uint8_t payload_len;
    uint8_t payload[128];
    uint16_t checksum;
    uint8_t end_byte;
} UART_ChunkReceive_Layout_t;

typedef enum
{
    UART_IDLE = 0,
    UART_START_BYTE,
    UART_PAYLOAD_LEN,
    UART_PAYLOAD,
    UART_CHECKSUM,
    UART_END_BYTE,
} UART_Chunks_States_t;

typedef enum
{
    UART_RECEPTION = 0,
    UART_CHECKING
} UART_Reception_States_t;

typedef enum
{
    UART_OK = 0,
    UART_OVERFLOW_ABORT,
    UART_RETRIES_ABORT,
    UART_CORRUPTED,
    UART_FLASH_ERROR
} UART_ChunkReceive_ReturnTypes_t;

typedef enum
{
    UART_ACK_OK = 0,
    UART_NACK_RETRY,
    UART_NACK_ABORT,
    UART_NACK_CORRUPTED,
    UART_NACK_OVERFLOW,
    UART_NACK_FLASH
} UART_HostConfirmation_t;

typedef enum
{
    SP_Validation_OK = 0,
    SP_Validation_ERROR
} SP_Validation_t;

typedef struct
{
    volatile uint32_t PC;
    volatile uint32_t R0;
    volatile uint32_t R1;
    volatile uint32_t R2;
    volatile uint32_t R3;
    volatile uint32_t R12;
    volatile uint32_t LR;
    volatile uint32_t xPSR;
    volatile uint32_t CFSR;
    volatile uint32_t HFSR;
    volatile uint32_t MMFAR;
    volatile uint32_t BFAR;
    bool mmfar_valid;
    bool bfar_valid;
    bool hfsr_forced;
} HardFault_Struct_t;

#endif // FLASH_CONFIG