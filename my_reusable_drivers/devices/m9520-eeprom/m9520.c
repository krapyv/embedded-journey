#include <stdio.h>
#include "m9520.h"
#include "spi.h"

void eeprom_write_enable()
{
    uint8_t wren_instr = WREN_INSTRUCTION;
    spi_transfer(&wren_instr, NULL, NULL, 1U, 1U, 0U);
}

void eeprom_wip_poll()
{
    uint8_t rdsr_instr = RDSR_INSTRUCTION;

    // WIP bit - bit 0 in Status Register
    // WIP bit indicates whether the memory is busy with a Write or Write Status Register cycle. When set to 1, such a cycle is in progress, when reset to 0, no such cycle is in progress

    // we need to poll the Status Register at least once
    // so do while is the best way to do it

    // the Read Status Register supports a continuous reading, but my implementation of the spi transfer requires tx/rx arrays or nulls
    // the tx array has RDSR instruction, so it is not NULL
    // in that case I need to have the TX array with the length of how many times the RDSR will be read continuously

    // instead of continuous reading, the poll will issue new transfer until WIP is 0 (no cycle is in progress)

    uint8_t transfer_length = 2; // instruction + status register out
    uint8_t status_rx;

    do
    {
        spi_transfer(&rdsr_instr, NULL, &status_rx, 2U, 1U, 1U);
    } while (status_rx & (1 << 0));

    return;
}

void eeprom_read(uint8_t *address, uint8_t *rx, uint16_t len)
{
    // check the len value against 4K-byte Memory array
    if ((len > 4096) || (len == 0))
    {
        return;
    }
    // len is the length of bytes about to be read

    // Read from Memory Array contains of:
    // an instruction (8 clock pulses):
    // MOSI - instruction bits, MISO - garbage

    // one 16-bit address (16 clock pulses)
    // MOSI - address bits, MISO - garbage

    // EEPROM output:
    // Case 1: if there is only one 8-bit read byte from EEPROM (8 clock pulses):
    // MOSI - garbage, MISO - 8 bits of read data
    // Chip Select must be driven high after the rising edge of Serial Clock for the last 8th bit of the instruction and before the next rising edge of Serial CLock

    // Case 2: len != 1
    // if Chip Select continues to be driven low, the internal address register is incremented automatically, and the byte of data at the new address is shifted out
    // when the highest address is reached, the address counter rolls over to zero, allowing the Read cycle to be continued indefinitely
    // The Read cycle is terminated after the len bytes were read

    // since we are going to pass the TX array of an instruction and address bytes to the spi_transfer()
    // we need to create a 3 byte TX array with [instruction, address]

    uint16_t transfer_length = len + (uint16_t)3;
    uint8_t tx[3];

    tx[0] = READ_INSTRUCTION;
    tx[1] = address[0];
    tx[2] = address[1];

    spi_transfer(tx, NULL, rx, transfer_length, 3U, 3U);

    return;
}

void eeprom_write_byte(uint8_t *address, uint8_t *data_payload, uint8_t len)
{
    // len is the length of bytes about to be written (data_payload)
    // address is an array with the length of 2

    // first of all, the WRITE operation requires the Write Enable operation first
    // if the WEL bit (Write Enable Latch) is not set to 1, the Write instruction is not accepted

    if ((len > 32) || (len == 0))
    {
        return;
    }

    eeprom_write_enable();

    // 3 more bytes: instruction + 2 address bytes
    uint8_t transfer_length = len + (uint8_t)3;

    uint8_t tx[3];

    tx[0] = WRITE_INSTRUCTION;
    tx[1] = address[0];
    tx[2] = address[1];

    spi_transfer(tx, data_payload, NULL, transfer_length, 3U, 0U);

    eeprom_wip_poll();

    return;
}
