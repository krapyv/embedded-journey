#include <stdio.h>
#include <stdint.h>

// extract_bits
uint32_t extract_bits(uint32_t value, unsigned int high, unsigned int low)
{
    // we need to find width (high - low + 1) (e.g. high = 4, low = 2 => width = 4 - 2 + 1 = 3 (bits 2, 3, 4)). without -1 the set would have been incomplete
    unsigned int width = high - low + 1;

    // next we need to create a mask that keeps all target bits and clears all other set bits outside the target range
    // it is basically the clear mask we are using in replace_bits, except for the lack of the last " << low", because we do not want to shift the target bits in the mask to the high - low position
    // we do not want to shift the ones in the mask for the bits extraction because in the extraction of bits we will shift the bits to the lowest bits in the value itself (e.g. 0b00111000 = 0b00000111)
    uint32_t mask = (1 << width) - 1; // - 1 gives us width of ones, not just one bit of one like it is after (1 << width)

    // 1. value >> low shifts the target range to the lowest bits (e.g. 0b0110100, low = 4, high = 5 => 0b0110100 >> 4 = 0b00000011)
    // 2. (value >> low) & mask - since AND (&) gives us 1 only when two inputs are 1, otherwise - 0, we are going to protect the target bits and to clear other possible higher bits
    // (e.g. 0b00011011 (after shifting right by low), we want the lowest 2 bits. mask = (0b00000001 << 2) - 1 = 0b00000100 - 1 = 4 - 1 = 3 = 0b00000011)
    // 0b00011011 & 0b00000011 = 0b00000011 = 3
    return (value >> low) & mask;
}

void replace_bits(uint32_t *value, unsigned int high, unsigned int low, uint32_t new_bits)
{
    unsigned int width = high - low + 1;

    // clearning mask
    // ((1 << width) - 1) creates the sequence of 1s of the length width (- 1 is a crucial part - it from only one 1 creates the width of 1s)
    // (((1 << width) - 1) << low) shifts the range of 1s to the target position (the lowest one is going to be at the position of low, the highest bit is going to be at the position high)
    // NOT (~) inverts the mask. it means ones become zeros, zeros become ones. it is used at the next step to clear the target bits in the value itself and protect the other set bits outside the range
    uint32_t clear_mask = ~(((1 << width) - 1) << low);

    // dereferencing the pointer value to manipulate on the value, not the address
    *value &= clear_mask;

    // now we need to "make up" the new_bits
    // new_bits should contain only the bits that are going to end up in *value. other set bits in new_bits are going to shift to the unwanted places and corrup the value
    // so, we are creating the mask to keep the target range of bits and to clear other ones
    uint32_t new_bits_mask = (((1 << width) - 1) << low); // the same mask as clear_bits but without NOT(~), because we do not want to clear the target range but protect it and clear the set bits outside the range

    // we are again working on the value itself, not the address of the pointer value, so we are dereferencing the value pointer
    // we are using OR (|) to keep all set bits in the *value (that already has been cleared from the set bits in the target range) and to "fuse" the bits from new_bits with the *value on the target positions
    // we are firstly "calculating" (new_bits & new_bits_mask) to obtain the "proper" new_bits (that has been cleared from any set bits outside the range that we want to replace the *value (high - low) range with)
    *value |= (new_bits & new_bits_mask);
}