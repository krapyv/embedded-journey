#ifndef FLASH_CONFIG
#define FLASH_CONFIG

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

#endif // FLASH_CONFIG