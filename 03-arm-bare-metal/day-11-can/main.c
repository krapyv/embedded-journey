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

    /* LOOPBACK TEST BENCH */

    // test id
    // since it is 11-bit long, uint16_t is the type that can hold the value without truncation
    // bits 15-12 are zeroed out
    uint16_t id = 0x1F2;

    uint8_t SIDH = id >> 3;

    // EXIDE and EID bits left at 0 for standard frames
    uint8_t SIDL = (id & 0x7) << 5;

    uint8_t bytes_number = 5;

    // bit 7: unimplemented, 0
    // bit 6: RTR, in this case for a data frame, 0
    // bits 5-4: unimplemented, 0
    // bits 3-0: number of data bytes to be transmitted (0-8 bytes)
    // 5 bytes = 0b0101 => in 8 bit representation: 0000 0101 (RTR explicitly cleared with 0)
    uint8_t DLC = bytes_number;

    // SIDH, SIDL, EID8 (zeroed out, don't care), EID0 (zeroed out, don't care), DLC, 5 data bytes
    uint8_t data_payload[10] = {SIDH, SIDL, 0x00, 0x00, DLC, 0x3F, 0xF1, 0xB4, 0x11, 0xAA};
    uint8_t load_tx_buffer_length = 10;

    mcp2515_load_tx_buffer(MCP_Load_TXB0SIDH, data_payload, load_tx_buffer_length);

    MCP_RTS_locations_t location = MCP_RTS_TXB0;

    mcp2515_rts(&location, 1U);

    // poll the RX0IF in READ_STATUS
    // RX0IF - receive buffer 0 Full Interrupt Flag bit
    // when RX0IF is 1 - interrupt is pending (must be cleared by MCU to reset the interrupt condition)
    uint8_t status_val = 0;

    do
    {
        mcp2515_read_status(&status_val, 1U);
    } while (!(status_val & (1 << 0)));

    // once RX buffer has the data frame, retrieve it
    uint8_t rx_frame_bytes[10];

    mcp2515_read_rx_buffer(MCP_Read_RXB0SIDH, rx_frame_bytes, 10U);
    // the READ RX BUFFER instruction automatically clears the associated receive flag, RXnIF (CANINTF), when CS is raised at the end of the command
    // so the RX0IF flag will be cleared automatically, so the hardware condition to reset the interrupt condition is satisfied

    uint8_t comparison_results[10];

    for (uint8_t i = 0; i < 10; i++)
    {
        // special case for SIDL since the datasheet does not guarantee the EID bits read as a defined value
        if (i == 1)
        {
            // 0x7 = 0111 (bits 7-5)
            uint8_t masked_rx_sidl = rx_frame_bytes[i] & (0x7 << 5);
            uint8_t masked_tx_sidl = data_payload[i] & (0x7 << 5);

            if (masked_rx_sidl == masked_tx_sidl)
            {
                comparison_results[i] = 0;
            }
            else
            {
                comparison_results[i] = 1;
            }

            continue;
        }
        // special case for EID8 and EID0 (since these bytes do not matter, we can omit them)
        if (i == 2 || i == 3)
        {
            comparison_results[i] = 0;
            continue;
        }
        // special case for DLC that differs between TX and RX
        if (i == 4)
        {
            // 1 << 6 (bit 6, RTR)
            // 0xF = 1111 (bits 3-0)
            uint8_t masked_rx_dlc = rx_frame_bytes[i] & ((1 << 6) | (0xF << 0));
            uint8_t masked_tx_dlc = data_payload[i] & ((1 << 6) | (0xF << 0));

            if (masked_tx_dlc == masked_rx_dlc)
            {
                comparison_results[i] = 0;
            }
            else
            {
                comparison_results[i] = 1;
            }

            continue;
        }
        if (rx_frame_bytes[i] == data_payload[i])
        {
            comparison_results[i] = 0;
        }
        else
        {
            comparison_results[i] = 1;
        }
    }
    return 0;
}