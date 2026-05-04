#ifndef BITWISE_H
#define BITWISE_H

#include <stdint.h>

// single bit operations
void set_bit(uint32_t *value, unsigned int n);
void clear_bit(uint32_t *value, unsigned int n);
void toggle_bit(uint32_t *value, unsigned int n);
int is_bit_set(uint32_t value, unsigned int n);

// multiple bit operations
uint32_t extract_bits(uint32_t value, unsigned int high, unsigned int low);
void replace_bits(uint32_t *value, unsigned int high, unsigned int low, uint32_t new_bits);

#endif
