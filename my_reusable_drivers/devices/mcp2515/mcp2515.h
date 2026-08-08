#ifndef MCP2515_H
#define MCP2515_H

#include <stdint.h>
#include "spi.h"
#include "core_cm4.h"

extern volatile uint8_t can_int_flag;

typedef enum
{
    MCP_CANINTF_RX0IF = (1 << 0),
    MCP_CANINTF_RX1IF = (1 << 1),
    MCP_CANINTF_TX0IF = (1 << 2),
    MCP_CANINTF_TX1IF = (1 << 3),
    MCP_CANINTF_TX2IF = (1 << 4),
    MCP_CANINTF_ERRIF = (1 << 5),
    MCP_CANINTF_WAKIF = (1 << 6),
    MCP_CANINTF_MERRF = (1 << 7)
} MCP_CANINTF_Masks_t;

typedef enum
{
    MCP_EFLG_RX1OVR = (1 << 7),
    MCP_EFLG_RX0OVR = (1 << 6)
} MCP_EFLG_Masks_t;

typedef enum
{
    MCP_Normal_Operation_Mode = (0 << 5),
    MCP_Sleep_Mode = (0x1 << 5),
    MCP_Loopback_Mode = (0x2 << 5), // 010
    MCP_Listen_Only_Mode = (0x3 << 5),
    MCP_Configuration_Mode = (0x4 << 5),
} MCP_Operation_Modes_t;

// 1001 0nm0 <= bits 2 and 1
typedef enum
{
    MCP_Read_RXB0SIDH = (0 << 1),   // 0x61
    MCP_Read_RXB0D0 = (0x1 << 1),   // 0x66
    MCP_Read_RXB1SIDH = (0x2 << 1), // 0x71
    MCP_Read_RXB1D0 = (0x3 << 1),   // 0x76
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

// CAN register addresses
static const uint8_t CANSTAT1 = 0x0E;
static const uint8_t CANCTRL1 = 0x0F; // 0000 1111 = 2^3 + 2^2 + 2^1 + 2^0 = 8 + 4 + 2 + 1 = 15 = 0x0F
static const uint8_t CNF3 = 0x28;
static const uint8_t CANINTE = 0x2B;
static const uint8_t CANINTF = 0x2C;
static const uint8_t EFLG = 0x2D;
static const uint8_t TXB0SIDH = 0x31;
static const uint8_t TXB0SIDL = 0x32;
static const uint8_t RXB0SIDH = 0x61;
static const uint8_t RXB0SIDL = 0x62;

// function headers
void mcp2515_init();
void mcp2515_canintf_handler(uint8_t can_intf_val, uint8_t *rx_buffer0, uint8_t *rx_buffer0_set, uint8_t *rx_buffer1, uint8_t *rx_buffer1_set);
void mcp2515_poll_bit_timeout(uint8_t addr, uint8_t mask, uint8_t expected_value, uint32_t timeout_ms, uint8_t *isSuccess);
void mcp2515_reset();
void mcp2515_read(uint8_t addr, uint8_t *rx_buffer, uint16_t read_len);
void mcp2515_read_rx_buffer(MCP_Read_RX_locations_t location_mask, uint8_t *rx_buffer, uint16_t read_len);
void mcp2515_write(uint8_t addr, uint8_t *data_payload, uint16_t write_len);
void mcp2515_load_tx_buffer(MCP_Load_TX_locations_t location_mask, uint8_t *data_payload, uint16_t write_len);
void mcp2515_rts(MCP_RTS_locations_t *locations, uint8_t locations_len);
void mcp2515_read_status(uint8_t *rx, uint16_t read_len);
void mcp2515_rx_status(uint8_t *rx, uint16_t read_len);
void mcp2515_bit_modify(uint8_t addr, uint8_t mask, uint8_t data_byte);

#endif // MCP2515_H