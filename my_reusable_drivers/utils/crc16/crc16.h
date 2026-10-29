#ifndef CRC16_H
#define CRC16_H

#include <stdint.h>

// CRC-16/X-25, check value: 0x906E for "123456789"

uint16_t crc16_bit(const uint8_t *data, uint16_t len);
uint16_t crc16_table(const uint8_t *data, uint16_t len);

#endif // CRC16_H