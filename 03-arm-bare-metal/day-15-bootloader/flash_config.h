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
} UART_ChunkReceive_t;

#endif // FLASH_CONFIG