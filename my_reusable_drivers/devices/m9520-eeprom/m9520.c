#include <stdio.h>
#include "m9520.h"
#include "spi.h"

void eeprom_write_enable()
{
    uint8_t wren_instr = WREN_INSTRUCTION;
    spi_transfer(&wren_instr, NULL, 1U);
}

void eeprom_read(uint8_t *address, uint8_t *result, uint8_t len)
{
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

    uint8_t read_instr = READ_INSTRUCTION;

    // since we are going to pass the TX array of an instruction and address bytes to the spi_transfer()
    // we need to create a len-length TX array with [instruction, address, dummy bytes ...]

    uint8_t tx[len];

    tx[0] = READ_INSTRUCTION;
    tx[1] = result[0];
    tx[2] = result[1];

    for (uint8_t i = 3; i < len; i++)
    {
        tx[i] = (uint8_t)0x00;
    }

    spi_transfer(tx, result, len);
}

void eeprom_write_byte()
{
}

void eeprom_wip_poll()
{
    uint8_t rdsr_instr = RDSR_INSTRUCTION;

    // WIP bit - bit 0 in Status Register
    // WIP bit indicates whether the memory is busy with a Write or Write Status Register cycle. WHen set to 1, such a cycle is in progress, when reset to 0, no such cycle is in progress
}
