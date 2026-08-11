#include <stdint.h>
#include <stdio.h>
#include "mcp2515.h"
#include "systick.h"

// INT ISR
void EXTI15_10_IRQHandler(void)
{
    // first of all, we need to know which exactly EXTI line caused the ISR to enter
    // EXTI Line 15 corresponds with EXIT_PR bit 15 set to 1

    if (EXTI->PR & (1 << 15))
    {
        // clear the bit 15 in EXTI_PR
        EXTI->PR = (1 << 15); // write-1-to-clear
        // set the flag to 1
        // INT line dropped to 0 and caused the interrupt
        can_int_flag = 1;
    }

    return;
}

void mcp2515_canintf_handler(uint8_t can_intf_val, uint8_t *rx_buffer0_header, uint8_t *rx_buffer0_payload, uint8_t *rx_buffer0_set, uint8_t *rx_buffer1_header, uint8_t *rx_buffer1_payload, uint8_t *rx_buffer1_set)
{
    // ERRIF handling: RX0OVR and RX1OVR in EFLG

    if (can_intf_val & MCP_CANINTF_ERRIF)
    {
        uint8_t eflg_val = 0;
        mcp2515_read(EFLG, &eflg_val, 1U);

        // firstly, reset RX0OVR/RX10VR in EFLG
        if (eflg_val & MCP_EFLG_RX1OVR)
        {
            // mask: 1000 0000 = 2^7 = 128 = 0x80 - the bit 7th is allowed to change
            // data byte: 0000 0000 = 0x00 - the bit 7th set to 0 (RX10VR and RX0OVR are write-0-to-clear)
            mcp2515_bit_modify(EFLG, 0x80, 0x00);
        }
        if (eflg_val & MCP_EFLG_RX0OVR)
        {
            // mask: 0100 0000 = 2^6 = 64 = 0x40 - the bit 6th is allowed to change
            // data byte: 0000 0000 = 0x00 - the bit 6th set to 0 (RX10VR and RX0OVR are write-0-to-clear)
            mcp2515_bit_modify(EFLG, 0x40, 0x00);
        }

        // a fresh EFLG read
        mcp2515_read(EFLG, &eflg_val, 1U);

        // check for TXB0
        if (eflg_val & MCP_EFLG_TXBO)
        {
            // the bus experiences the bus-off error
            can_bus_off = 1;
        }
        // check for anything else set in EFLG outside RX1OVR, RX0OVR and TXBO
        if (eflg_val & ~(MCP_EFLG_RX1OVR | MCP_EFLG_RX0OVR | MCP_EFLG_TXBO))
        {
            // if there are some other unhandled error bits
            can_intf_stuck = 1;
        }

        // check if the EFLG is clear
        if (eflg_val == 0)
        {
            // if it is, clear the ERRIF in CANINTF
            // mask: 0010 0000 = 2^5 = 32 = 0x20 - the bit 5th is allowed to change
            // data byte: 0000 0000 = 0x00
            mcp2515_bit_modify(CANINTF, 0x20, 0x00);
        }
    }

    uint8_t dlc_value;

    // read the RXn buffer via READ RX BUFFER command
    // READ RX BUFFER automatically clears RXnIF in CANINTF
    if (can_intf_val & MCP_CANINTF_RX0IF)
    {
        // request the header bytes (SDIH to DLC)
        mcp2515_read_rx_buffer(MCP_Read_RXB0SIDH, rx_buffer0_header, 5);

        // 0000 1111 = 2^3 + 2^2 + 2^1 + 2^0 = 8 + 4 + 2 + 1 = 15 = 0x0F
        dlc_value = rx_buffer0_header[4] & 0x0F;

        // request the data bytes
        mcp2515_read_rx_buffer(MCP_Read_RXB0D0, rx_buffer0_payload, dlc_value);
        *rx_buffer0_set = 1;
    }
    if (can_intf_val & MCP_CANINTF_RX1IF)
    {
        // request the header bytes (SDIH to DLC)
        mcp2515_read_rx_buffer(MCP_Read_RXB1SIDH, rx_buffer1_header, 5);

        // 0000 1111 = 2^3 + 2^2 + 2^1 + 2^0 = 8 + 4 + 2 + 1 = 15 = 0x0F
        dlc_value = rx_buffer1_header[4] & 0x0F;

        // request the data bytes
        mcp2515_read_rx_buffer(MCP_Read_RXB1D0, rx_buffer1_payload, dlc_value);
        *rx_buffer1_set = 1;
    }
}

// the function that handles INT pin configuration
// on the STM32F411, pin PB15 is the GPIO pin connected with the INT line of the MCP2515
void mcp2515_init()
{
    // GPIOB RCC pin enablement
    RCC->AHB1ENR |= (1 << 1);

    // setting the pin PB15 to GPIO input mode
    // MODER value for Input is 00, since the MODER gives 2 bits per pin

    // to set the bits to 00, we need to clear them
    // pin 15 takes MODER[31:30]
    // 11 = 0x3
    GPIOB->MODER &= ~(0x3 << 30);

    // SYSCFG RCC pin enablement
    RCC->APB2ENR |= (1 << 14);

    // mux EXTI Line 15 in SYSCFG_EXTICRn to GPIOB
    // EXTI Line 15 is set in SYSCFG_EXTICR4
    // EXTI15 takes bits 15:12 (4 bits)

    // clear the range
    // 1111 = 2^3 + 2^2 + 2^1 + 2^0 = 8 + 4 + 2 + 1 = 15 = 0xF
    SYSCFG->EXTICR4 &= ~(0xF << 12);

    // set the value of 0001 to the range
    // PB[x] pin is the value of 0001
    SYSCFG->EXTICR4 |= (1 << 12);

    // trigger selection
    // since INT is active-low/open-drain, the transition from HIGH to LOW signals
    // so setting the bit 15 in the EXTI_FTSR register
    EXTI->FTSR |= (1 << 15);

    // unmask the interrupt request in EXTI_IMR
    EXTI->IMR |= (1 << 15);

    // NVIC Interrupt enablement
    // EXTI15_10 sits at position 40 in the Vector table

    // position 40 = 40 / 32 = ISER1
    // 40 % 32 = 8 - bit 8
    NVIC->ISER[1] = (1 << 8);
    // with NVIC that is a part of Cortex-M4, not the STM32 own core, we are using single write
}

void mcp2515_poll_bit_timeout(uint8_t addr, uint8_t mask, uint8_t expected_value, uint32_t timeout_ms, uint8_t *isSuccess)
{
    uint8_t reg_val;
    uint32_t start = SysTick_GetTick();
    *isSuccess = 0;

    do
    {
        mcp2515_read(addr, &reg_val, 1U);

        if ((reg_val & mask) == expected_value)
        {
            *isSuccess = 1;
        }
    } while (SysTick_GetTick() - start <= timeout_ms && *isSuccess != 1);

    return;
}

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