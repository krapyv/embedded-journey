#include <stdint.h>
#include <stdio.h>
#include "mcp2515.h"
#include "systick.h"

void mcp2515_reset()
{
    // RESET is used to reinitialize the internal registers and set the Configuration mode

    // Requires selecting the device by pulling the CS pin low, sending the instruction byte and then raising the CS pin

    uint8_t settings_tx = RESET_INSTRUCTION;

    spi_transfer(&settings_tx, NULL, NULL, 1, 1, 1);

    // the Oscillator Start-up Timer keeps the device in Reset for 128 OSC1 clock cycles after an SPI Reset, and no SPI protocol operations should be attempted until the OST has expired
    SysTick_Delay_ms(2);
}

void mcp2515_read(uint8_t addr, uint8_t *rx_buffer, uint16_t read_len)
{
    // The READ is started by lowering the CS pin
    // Then the instruction is sent to the MCP2515, followed by the 8-bit  address
    // Next, the data stored in the register at the selected address will be shifted out on the SO pin
    // It is possible to read the next consecutive register address by continuing to provide clock pulses
    // The READ operation is terminated by raising the CS pin

    uint8_t settings_buffer[2] = {READ_INSTRUCTION, addr};

    uint16_t total_length = read_len + (uint16_t)2;

    spi_transfer(settings_buffer, NULL, rx_buffer, total_length, 2U, 2U);
}

void mcp2515_read_rx_buffer(MCP_Read_RX_locations_t location_mask, uint8_t *rx_buffer, uint16_t read_len)
{
    // Provides a way to quickly address a receive buffer for reading
    // Reduces the SPI overhead by one byte, the address byte
    // Once the command byte is sent, the controller clocks out the data at the address location
    // Sequential reads are possible
    // At the end of the command the CS pin is raised

    uint8_t read_rx_buff_instr = READ_RX_BUFFER_BASE | location_mask;
    uint16_t total_len = read_len + (uint16_t)1;

    spi_transfer(&read_rx_buff_instr, NULL, rx_buffer, total_len, 1U, 1U);
}

void mcp2515_write(uint8_t addr, uint8_t *data_payload, uint16_t write_len)
{
    // The WRITE instruction is started by lowering the CS pin
    // Then the instruction is sent to the MCP2515, followed by the address and at least one byte of data
    // It is possible to write to sequential registers by continuing to clock in data bytes as long as CS is held low

    uint8_t settings_tx[2] = {WRITE_INSTRUCTION, addr};
    uint16_t total_len = write_len + (uint16_t)2;

    spi_transfer(settings_tx, data_payload, NULL, total_len, 2U, 0U);
}

void mcp2515_load_tx_buffer(MCP_Load_TX_locations_t location_mask, uint8_t *data_payload, uint16_t write_len)
{
    // Eliminates the eight-bit address required by a normal WRITE command
    // The eight-bit instruction sets the Address Pointer to one of six addresses to quickly write to a transmit buffer that points to the "ID" or "data" address of any of the three transmit buffers

    uint8_t load_tx_buff_instr = LOAD_TX_BUFFER_BASE | location_mask;

    uint16_t total_len = write_len + (uint16_t)1;

    spi_transfer(&load_tx_buff_instr, data_payload, NULL, total_len, 1U, 0U);
}

void mcp2515_rts(MCP_RTS_locations_t *locations, uint8_t locations_len)
{
    if (locations_len > 3)
    {
        return;
    }
    // Used to initiate message transmission for one or more of the transmit buffers

    // The MCP2515 is selected by lowering the CS pin
    // The RTS command byte is then sent
    // The last three bits of this command indicate which transmit buffer(s) are enabled to send
    // If the RTS comman is sent with nnn = 000, the command will be ignored

    uint8_t rts_instr = RTS_BASE;

    for (uint8_t i = 0; i < locations_len; i++)
    {
        rts_instr |= locations[i];
    }

    spi_transfer(&rts_instr, NULL, NULL, 1U, 1U, 0U);
}

void mcp2515_read_status(uint8_t *rx, uint16_t read_len)
{
    // Allows single instruction access to some of the often used status bits for message reception and transmission

    // The MCP2515 is selected by lowering the CS pin
    // The comman byte is sent to the MCP2515
    // Once the comman byte is sent, the MCP2515 will return eight bits of data that contain the status

    // If additional clocks are sent after the first eight bits are transmitted, the MCP2515 will continue to output the status bits as long as the CS pin is held low and clocks are provided on SCK

    uint8_t settings_tx = READ_STATUS_INSTRUCTION;

    uint16_t total_len = read_len + (uint16_t)1;

    spi_transfer(&settings_tx, NULL, rx, total_len, 1U, 1U);
}

void mcp2515_rx_status(uint8_t *rx, uint16_t read_len)
{
    // Used to quickly determine which filter matched the message and message type

    // After the command byte is sent, the controller will return 8 bits of data that contain the status data
    // If more clocks are sent after the eight bits are transmitted the controller will continue to output the same status bits as long as the CS pin stays low and clocks are provided

    uint8_t settings_tx = RX_STATUS_INSTRUCTION;

    uint16_t total_len = read_len + (uint16_t)1;

    spi_transfer(&settings_tx, NULL, rx, total_len, 1U, 1U);
}

void mcp2515_bit_modify(uint8_t addr, uint8_t mask, uint8_t data_byte)
{
    // Provides a means for setting or clearing individual bits in specific status and control registers

    // The part is selected by lowering the CS pin
    // The BIT MODIFY command byte is then sent to the MCP2515
    // The command is followed by the address of the register, the mask byte and the data byte

    // The mask byte determines which bits in the register will be allowed to change
    // A 1 in the mask byte will allow a bit in the register to change, 0 - will not

    // The data byte determines what value the modified bits in the register will be changed to
    // A 1 in the data byte will set the bit and 0 will clear the bit, provided that the mask for that bit is set to a 1

    uint8_t settings_tx[2] = {BIT_MODIFY_INSTRUCTION, addr};
    uint8_t data_payload[2] = {mask, data_byte};

    spi_transfer(settings_tx, data_payload, NULL, 4U, 2U, 0U);
}