#include <stdio.h>
#include <stdint.h>

// single bit operations
void set_bit(uint32_t *value, unsigned int n)
{
    // 0b00011001, n = 2
    // 1 << n = 0b00000001 << 2 = 0b00000100
    // set_bit: *value |= mask (because we want to keep all existing set bits in the *value and to set a new one using the mask) (OR => 1 0 = 1; 0 1 = 1; 1 1 = 1; 0 0 = 0)
    // 0b00011001 | 0b00000100 = 0b00011101
    *value |= (1 << n);
}
void clear_bit(uint32_t *value, unsigned int n)
{
    // 0b00011001, n = 3
    // mask = ~(1 << n) - becasue we want to invert the mask to make the target bit as 0 and all other bits as 1
    // because the next step is *value &= mask => AND (&) gives us 1 only when two inputs are 1 (1 1 = 1; 0 1 = 0; 1 0 =0; 0 0 = 0)
    // so all 1s in the value other than at the position of n will be kept, but the one at the position of n will be replaced by 0
    // ~(1 << n) = ~(0b00000001 << 3) = ~0b00001000 = 0b11110111
    // *value = 0b00011001 & 0b11110111 = 0b00010001
    *value &= ~(1 << n);
}

void toggle_bit(uint32_t *value, unsigned int n)
{
    // 0b00011001, n = 2
    // 1 << n = 0b00000001 << 2 = 0b00000100

    // we are toggling bits using XOR (1 0 = 1; 0 1 = 1; 1 1 = 0; 0 0 = 0) (gives us 1 only when two input differ)
    // so the formula is *value = *value ^ (1 << n)

    // 0b00011001 ^ 0b00000100 = 0b00011101

    *value ^= (1 << n);
}

int is_bit_set(uint32_t value, unsigned int n)
{
    // to find whether the bit at the position of n is set, we have two ways
    // (value >> n) & 1
    // (value & (1 << n)) != '0'

    // 1. value = 0b00011101, n = 3
    // (value >> 3) & 0b00000001 = 0b00000011 & 0b00000001 = 0b00000001 = 1 // set

    // value = 0b00011101, n = 1
    // (value >> 1) & 0b00000001 = 0b00001110 & 0b00000001 = 0b00000000 = 0 // not set

    // 2. value = 0b00011101, n = 3
    // value & (1 << n) = 0b00011101 & (1 << 3) = 0b00011101 & (0b00000001 << 3) = 0b00011101 & 0b00001000 = 0b00001000 = 8 != 0 // set

    // value = 0b00011101, n = 1
    // value & (1 << n) = 0b00011101 & (0b00000001 << 1) = 0b00011101 & 0b00000010 = 0b00000000 = 0 == 0 // not set

    return (value >> n) & 1;
}

// multiple bit operations
uint32_t extract_bits(uint32_t value, unsigned int high, unsigned int low)
{
    unsigned int width = high - low + 1;
    uint32_t mask = (1 << width) - 1;
    return (value >> low) & mask;
}

void replace_bits(uint32_t *value, unsigned int high, unsigned int low, uint32_t new_bits)
{
    unsigned int width = high - low + 1;

    uint32_t clear_mask = ~(((1 << width) - 1) << low);

    *value &= clear_mask;

    uint32_t new_bits_mask = (((1 << width) - 1) << low);

    *value |= (new_bits & new_bits_mask);
}