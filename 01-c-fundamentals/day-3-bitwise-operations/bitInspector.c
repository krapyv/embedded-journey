#include <stdio.h>
#include <stdint.h>
// well, we need to print out binary
// https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format
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

// first attempt
// void set_bit(uint32_t value, int n)
// {
//     int mask = 1 << n;
//     uint32_t v = value | mask;

//     printf("Set bit %d of a %d (" BYTE_TO_BINARY_PATTERN ") to 1: res = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", n, value, BYTE_TO_BINARY(value), v, BYTE_TO_BINARY(v));
// }

// int is_bit_set(uint32_t value, int n)

void set_bit(uint32_t *value, int n)
{
    *value = *value | (1 << n); // (1 << n) is a mask
}

// first attempt
// as i understand, this function should remove bit 1 from the position or does nothing if the bit is already 0
// void clear_bit(uint32_t *value, int n)
// {
//     // for example:
//     // i have 13: 0b00001101 (2^3 + 2^2 + 2^0) = 8 + 4 + 1 = 13
//     // i want to clear the bit 3 (n = 3)
//     // i could use mask 1 << n => 0b00000001 << 3 => 0b00001000
//     // then i could use XOR because we have in our number 13 (0b00001101) and mask (0b00001000) common bit 3 that is 1
//     // XOR gives us 1 when two inputs differ (e.g. 1 and 0) and gives 0 when they are the same (0 and 0 or 1 and 1)
//     // we have in our number and mask only one identical bit, so we would "clear it"
//     // 0b00001101 ^ 0b00001000 = 0b00000101 = 5
//     *value = *value ^ (1 << n);
// }

void clear_bit(uint32_t *value, int n)
{
    *value = *value & ~(1 << n);
}

void toggle_bit(uint32_t *value, int n)
{
    *value = *value ^ (1 << n);
}

int is_bit_set(uint32_t value, int n)
{
    return (value >> n) & 1;
    // value = 5, n = 2;
    // (0b00000101 >> 2) & 0b00000001 = 0b00000001 & 0b00000001 = 1 - bit is set

    // value = 5, n = 3;
    // (0b00000101 >> 3) & 0b00000000 = 0b00000000 & 0b00000001 = 0 - bit is not set

    // ok, that makes sense
}

int main(void)
{
    // uint32_t value = 4;
    // unsigned int n = 1;
    // printf("before: %d, binary = " BYTE_TO_BINARY_PATTERN "\n", value, BYTE_TO_BINARY(value));
    // set_bit(&value, 1);
    // printf("after setting of a bit %d to 1: %u, binary = " BYTE_TO_BINARY_PATTERN "\n", n, value, BYTE_TO_BINARY(value));

    uint32_t value = 5;
    unsigned int n = 4;
    // printf("before: %d, binary = " BYTE_TO_BINARY_PATTERN "\n", value, BYTE_TO_BINARY(value));
    // clear_bit(&value, n);
    // printf("after clearning a bit %d: %u, binary = " BYTE_TO_BINARY_PATTERN "\n", n, value, BYTE_TO_BINARY(value));

    // printf("before: %d, binary = " BYTE_TO_BINARY_PATTERN "\n", value, BYTE_TO_BINARY(value));
    // toggle_bit(&value, n);
    // printf("after toggling a bit %d: %u, binary = " BYTE_TO_BINARY_PATTERN "\n", n, value, BYTE_TO_BINARY(value));

    printf("before: %d, binary = " BYTE_TO_BINARY_PATTERN "\n", value, BYTE_TO_BINARY(value));
    printf("The bit %d is set? (1 - true, 0 - false): %d\n", n, is_bit_set(value, n));

    return 0;
}