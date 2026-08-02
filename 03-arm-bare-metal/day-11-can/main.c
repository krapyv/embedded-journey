#include "spi.h"
#include "mcp2515.h"
#include "systick.h"

int main(void)
{
    SysTick_Init(SYSTICK_FREQUENCY_16MHZ);
    spi_init(SPI_BR_8);
    mcp2515_reset();

    uint8_t rx_byte = 0;

    mcp2515_read(CANSTAT1, &rx_byte, 1U);

    // CNF3, CNF2, CNF1
    uint8_t CNF_vals[3] = {0x01, 0x91, 0x00};

    // CNF3 = 0x28, CNF2 = 0x29, CNF1 = 0x2A
    // since all three registers are consecutive, we can write them in one write operation
    mcp2515_write(CNF3, CNF_vals, 3U);

    uint8_t read_CNF_vals[3];

    mcp2515_read(CNF3, read_CNF_vals, 3U);

    uint8_t CANCTRL_val;

    mcp2515_read(CANCTRL1, &CANCTRL_val, 1);

    // (1 << 7) | MCP_Loopback_Mode = (1 << 7) | (0x2 << 5)
    // mask: 1100 0000
    // current value: 1000 xxxx
    uint8_t mask = (1 << 7) | MCP_Loopback_Mode;
    uint8_t data_byte = MCP_Loopback_Mode;

    mcp2515_bit_modify(CANCTRL1, mask, data_byte);

    uint8_t read_CANSTAT_val;
    uint8_t read_CANCTRL_val;

    mcp2515_read(CANSTAT1, &read_CANSTAT_val, 1);
    mcp2515_read(CANCTRL1, &read_CANCTRL_val, 1);

    return 0;
}