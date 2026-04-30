#include <stdio.h>
#include <stdint.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)         \
    ((byte) & 0x80 ? '1' : '0'),     \
        ((byte) & 0x40 ? '1' : '0'), \
        ((byte) & 0x20 ? '1' : '0'), \
        ((byte) & 0x10 ? '1' : '0'), \
        ((byte) & 0x08 ? '1' : '0'), \
        ((byte) & 0x04 ? '1' : '0'), \
        ((byte) & 0x02 ? '1' : '0'), \
        ((byte) & 0x01 ? '1' : '0')

void set_bit(uint32_t *value, int n)
{
    *value |= (1 << n);

    // for example, *value = 3; n = 2;
    // 3 = 0b00000011 = 2^1 + 2^0
    // *value = 0b00000011 | (0b00000001 << 2) = 0b00000011 | 0b00000100 = 0b00000111 = 2^2 + 2^1 + 2^0 = 7
}

void clear_bit(uint32_t *value, int n)
{
    *value &= ~(1 << n);

    // *value = 3, n = 1;
    // *value = 0b00000011 & ~(0b00000001 << 1) = 0b00000011 & ~(0b00000010) = 0b00000011 & 0b11111101 = 0b00000001 = 1;
}

void toggle_bit(uint32_t *value, int n)
{
    *value ^= (1 << n);

    // *value = 3, n =4;
    // *value = 0b00000011 ^ (0b00000001 << 4) = 0b00000011 ^ 0b00010000 = 0b00010011 (XOR gives us 1 when two inputs differ (0 and 1 or 1 and 0) and gives us 0 when two inputs are the same (1 and 1 or 0 and 0))
}

int is_bit_set(uint32_t value, int n)
{
    return (value >> n) & 1; // or value & (1 << n) != 0;
    // value = 3; n = 3;
    // (0b00000011 >> 3) & 0b00000001 = 0b00000000 & 0b00000001 = 0 // bit is not set
    // value = 3; n = 1;
    // (0b00000011 >> 1) & 0b00000001 = 0b00000001 & 0b00000001 = 1 // bit is set
}

int main(void)
{
    uint32_t value1 = 4;
    unsigned int n1 = 1;
    printf("before: %d, binary = " BYTE_TO_BINARY_PATTERN "\n", value1, BYTE_TO_BINARY(value1));
    set_bit(&value1, n1);
    printf("after setting of a bit %d to 1: %u, binary = " BYTE_TO_BINARY_PATTERN "\n", n1, value1, BYTE_TO_BINARY(value1));

    uint32_t value2 = 5;
    unsigned int n2 = 4;
    printf("before: %d, binary = " BYTE_TO_BINARY_PATTERN "\n", value2, BYTE_TO_BINARY(value2));
    clear_bit(&value2, n2);
    printf("after clearning a bit %d: %u, binary = " BYTE_TO_BINARY_PATTERN "\n", n2, value2, BYTE_TO_BINARY(value2));

    uint32_t value3 = 6;
    unsigned int n3 = 2;
    printf("before: %d, binary = " BYTE_TO_BINARY_PATTERN "\n", value3, BYTE_TO_BINARY(value3));
    toggle_bit(&value3, n3);
    printf("after toggling a bit %d: %u, binary = " BYTE_TO_BINARY_PATTERN "\n", n3, value3, BYTE_TO_BINARY(value3));

    printf("before: %d, binary = " BYTE_TO_BINARY_PATTERN "\n", value3, BYTE_TO_BINARY(value3));
    printf("The bit %d is set? (1 - true, 0 - false): %d\n", n3, is_bit_set(value3, n3));
}