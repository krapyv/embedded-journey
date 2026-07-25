#ifndef M9520_H
#define M9520_H

#include <stdint.h>

// Write Enable
#define WREN_INSTRUCTION 0x6 // 0000 0110 = 2^2 + 2^1 = 4 + 2 = 6
// Write Disable
#define WRDI_INSTRUCTION 0x4 // 0000 0100 = 2^2 = 4
// Read Status Register
#define RDSR_INSTRUCTION 0x5 // 0000 0101 = 2^2 + 2^0 = 4 + 1 = 5
// Write Status Register
#define WRSR_INSTRUCTION 0x1 // 0000 0001 = 2^0 = 1
// Read from Memory Array
#define READ_INSTRUCTION 0x3 // 0000 0011 = 2^1 + 2^0 = 2 + 1 = 3
// Write to Memory Array
#define WRITE_INSTRUCTION 0x2 // 0000 0010 = 2^1 = 2

// function headers
void eeprom_write_enable();
void eeprom_read(uint8_t *address, uint8_t *rx, uint16_t len);
void eeprom_write_byte(uint8_t *address, uint8_t *data_payload, uint8_t len);
void eeprom_wip_poll();

#endif // M9520_H