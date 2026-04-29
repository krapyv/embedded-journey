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

// Write all 30 expressions with two and three combined operators I have done on paper to verify the results

void verifyExpressions()
{
    printf("1.(5 << 1) & 3, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (5 << 1) & 3, BYTE_TO_BINARY((5 << 1) & 3));
    printf("2.(3 << 2) | 1, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (3 << 2) | 1, BYTE_TO_BINARY((3 << 2) | 1));
    printf("3.(6 >> 1) ^ 2, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (6 >> 1) ^ 2, BYTE_TO_BINARY((6 >> 1) ^ 2));
    printf("4.(2 << 3) & 7, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (2 << 3) & 7, BYTE_TO_BINARY((2 << 3) & 7));
    printf("5.(8 >> 2) | 5, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (8 >> 2) | 5, BYTE_TO_BINARY((8 >> 2) | 5));
    printf("6.(4 << 2) ^ 10, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (4 << 2) ^ 10, BYTE_TO_BINARY((4 << 2) ^ 10));
    printf("7.(1 << 4) & 15, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (1 << 4) & 15, BYTE_TO_BINARY((1 << 4) & 15));
    printf("8.(20 >> 2) | 3, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (20 >> 2) | 3, BYTE_TO_BINARY((20 >> 2) | 3));
    printf("9.(9 >> 1) ^ 4, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (9 >> 1) ^ 4, BYTE_TO_BINARY((9 >> 1) ^ 4));
    printf("10.(0b1010 << 1) & 0b1111, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (0b1010 << 1) & 0b1111, BYTE_TO_BINARY((0b1010 << 1) & 0b1111));
    printf("11.(5 & 3) << 1, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (5 & 3) << 1, BYTE_TO_BINARY((5 & 3) << 1));
    printf("12.(7 | 1) >> 1, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (7 | 1) >> 1, BYTE_TO_BINARY((7 | 1) >> 1));
    printf("13.(6 ^ 4) << 2, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (6 ^ 4) << 2, BYTE_TO_BINARY((6 ^ 4) << 2));
    printf("14.(10 & 6) >> 1, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (10 & 6) >> 1, BYTE_TO_BINARY((10 & 6) >> 1));
    printf("15.(3 | 5) << 2, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (3 | 5) << 2, BYTE_TO_BINARY((3 | 5) << 2));
    printf("16.(12 ^ 5) >> 2, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (12 ^ 5) >> 2, BYTE_TO_BINARY((12 ^ 5) >> 2));
    printf("17.(15 & 10) << 1, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (15 & 10) << 1, BYTE_TO_BINARY((15 & 10) << 1));
    printf("18.(9 | 6) >> 1, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (9 | 6) >> 1, BYTE_TO_BINARY((9 | 6) >> 1));
    printf("19.(14 ^ 9) << 1, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (14 ^ 9) << 1, BYTE_TO_BINARY((14 ^ 9) << 1));
    printf("20.(0b1100 & 0b1010) >> 1, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (0b1100 & 0b1010) >> 1, BYTE_TO_BINARY((0b1100 & 0b1010) >> 1));
    printf("21.(5 & 3) | (2 << 1), result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (5 & 3) | (2 << 1), BYTE_TO_BINARY((5 & 3) | (2 << 1)));
    printf("22.(8 >> 2) ^ (3 << 1), result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (8 >> 2) ^ (3 << 1), BYTE_TO_BINARY((8 >> 2) ^ (3 << 1)));
    printf("23.(7 | 1) & (4 << 1), result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (7 | 1) & (4 << 1), BYTE_TO_BINARY((7 | 1) & (4 << 1)));
    printf("24.(10 & 7) >> 1 | 2, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (10 & 7) >> 1 | 2, BYTE_TO_BINARY((10 & 7) >> 1 | 2));
    printf("25.(6 ^ 3) << 1 & 10, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (6 ^ 3) << 1 & 10, BYTE_TO_BINARY((6 ^ 3) << 1 & 10));
    printf("26.(9 >> 2) | (5 & 3) << 1, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (9 >> 2) | (5 & 3) << 1, BYTE_TO_BINARY((9 >> 2) | (5 & 3) << 1));
    printf("27.(12 & 5) ^ (1 << 3) >> 1, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (12 & 5) ^ (1 << 3) >> 1, BYTE_TO_BINARY((12 & 5) ^ (1 << 3) >> 1));
    printf("28.~5 & (3 << 2), result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", ~5 & (3 << 2), BYTE_TO_BINARY(~5 & (3 << 2)));
    printf("29.(~3 & 7) | (4 >> 1), result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", (~3 & 7) | (4 >> 1), BYTE_TO_BINARY((~3 & 7) | (4 >> 1)));
    printf("30.((3 << 2) | 5) & 10, result = %d, binary = " BYTE_TO_BINARY_PATTERN "\n", ((3 << 2) | 5) & 10, BYTE_TO_BINARY(((3 << 2) | 5) & 10));
}

int main(void)
{
    verifyExpressions(); // all results are the identical to my paper ones
    return 0;
}