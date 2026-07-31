#ifndef MCP2515_H
#define MCP2515_H

#include <stdint.h>

// 1001 0nm0 <= bits 2 and 1
typedef enum
{
    MCP_Read_RX_00 = (0 << 1),
    MCP_Read_RX_01 = (0x1 << 1),
    MCP_Read_RX_10 = (0x2 << 1),
    MCP_Read_RX_11 = (0x3 << 1),
} MCP_Read_RX_locations_t;

// 0100 0abc <= bits 2, 1 and 0
typedef enum
{
    MCP_Load_TXB0SIDH = (0 << 0),
    MCP_Load_TXB0D0 = (0x1 << 0),
    MCP_Load_TXB1SIDH = (0x2 << 0),
    MCP_Load_TXB1D0 = (0x3 << 0),
    MCP_Load_TXB2SIDH = (0x4 << 0),
    MCP_Load_TXB2D0 = (0x5 << 0),
} MCP_Load_TX_locations_t;

// 100 0nnn <= bits 2, 1 and 0
typedef enum
{
    MCP_RTS_TXB2 = (1 << 2),
    MCP_RTS_TXB1 = (1 << 1),
    MCP_RTS_TXB0 = (1 << 0)
} MCP_RTS_locations_t;

// SPI instruction set

// RESET
static const uint8_t RESET_INSTRUCTION = 0xC0U; // 1100 0000 = 2^7 + 2^6 = 128 + 64 = 192 = 0xC0
// READ
static const uint8_t READ_INSTRUCTION = 0x3U; // 0000 0011 = 2^1 + 2^0 = 0x3
// WRITE
static const uint8_t WRITE_INSTRUCTION = 0x2U; // 0000 0010 = 2^1 = 0x2
// READ STATUS
static const uint8_t READ_STATUS_INSTRUCTION = 0xA0U; // 1010 0000 = 2^7 + 2^5 = 128 + 32 = 160 = 0xA0
// RX STATUS
static const uint8_t RX_STATUS_INSTRUCTION = 0xB0U; // 1011 0000 = 2^7 + 2^5 + 2^4 = 128 + 32 + 16 = 176 = 0xB0
// BIT MODIFY
static const uint8_t BIT_MODIFY_INSTRUCTION = 0x5U; // 0000 0101 = 2^2 + 2^0 = 5 = 0x5

static const uint8_t READ_RX_BUFFER_BASE = 0x90U; // 1001 0000 = 2^7 + 2^4 = 128 + 16 = 144 = 0x90
static const uint8_t LOAD_TX_BUFFER_BASE = 0x40U; // 0100 0000 = 2^6 = 64 = 0x40
static const uint8_t RTS_BASE = 0x80U;            // 1000 0000 = 2^7 = 128 = 0x80

#endif // MCP2515_H