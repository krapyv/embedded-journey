#include <stdio.h>
#include <stdint.h>

// it works!
// uint32_t extract_bits(uint32_t value, unsigned int high, unsigned int low)
// {
//     value >>= low;
//     // value = 25 => value = 0b00011001 = 2^4 + 2^3 + 2^0
//     // we want bits 3-1 (100)
//     // shift first =>  0b00001100

//     value &= (1 << (high - low + 1)) - 1;
//     // high - low + 1 = 3 - 1 + 1 = 3
//     // (1 << (high - low + 1)) - 1
//     // 0b00001100 & ((0b00000001 << 3) - 1) = 0b00001100 & (0b00001000 - 1) = 0b00001100 & 8 - 1 =  0b00001100 & 7 = 0b00001100 & 0b00000111 = 0b00000100 = 2^2 = 4

//     return value;
// }

#include <stdio.h>
#include <stdint.h>

uint32_t extract_bits(uint32_t value, unsigned int high, unsigned int low)
{
    unsigned int width = high - low + 1;
    uint32_t mask = (1 << width) - 1;

    return (value >> low) & mask;
}

void replace_bits(uint32_t *value, unsigned int high, unsigned int low, uint32_t new_bits)
{
    // clear the target bits in *value
    // *value = 25 => 0b00011001 => 2^4 + 2^3 + 2^0 = 25
    // high = 3, low = 1; new_bits = 0b1110 = 2^3 + 2^2 + 2^1 = 8 + 4 + 1 = 13;
    // we want to clear 3-1 bits in 0b00011001 => 100
    // we could clear it with AND (&) of the value and the value of 0b00010001
    // 0b00011001 & 0b00010001 = 0b00010001
    // then we want to shift new_bits to the correct position
    // let's say we have new_bits = 0b111 (positions 2-0)
    // we need to shift it to positions 3-1;
    // 0b111 << 1 => 0b1110 (positions 3-1)

    // after that we need to OR the shifted new_bits into the *value
    // *value = *value | new_bits = 0b00010001 | 0b00001110 = 0b00011111;
}

int main(void)
{
    uint32_t value = 25;
    unsigned int low = 1, high = 3;
    printf("extract bits %u-%u in number %u: %u\n", high, low, value, extract_bits(value, high, low)); // 4
}
