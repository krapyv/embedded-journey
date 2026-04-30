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

// my first thoughts
// void replace_bits(uint32_t *value, unsigned int high, unsigned int low, uint32_t new_bits)
// {
//     // clear the target bits in *value
//     // *value = 25 => 0b00011001 => 2^4 + 2^3 + 2^0 = 25
//     // high = 3, low = 1; new_bits = 0b1110 = 2^3 + 2^2 + 2^1 = 8 + 4 + 1 = 13;
//     // we want to clear 3-1 bits in 0b00011001 => 100
//     // we could clear it with AND (&) of the value and the value of 0b00010001
//     // 0b00011001 & 0b00010001 = 0b00010001
//     // then we want to shift new_bits to the correct position
//     // let's say we have new_bits = 0b111 (positions 2-0)
//     // we need to shift it to positions 3-1;
//     // 0b111 << 1 => 0b1110 (positions 3-1)

//     // after that we need to OR the shifted new_bits into the *value
//     // *value = *value | new_bits = 0b00010001 | 0b00001110 = 0b00011111;
// }

void replace_bits(uint32_t *value, unsigned int high, unsigned int low, uint32_t new_bits)
{
    // first of all, we need to create a mask that clears the bits we want to replace in the value

    // for that we need to find how many bits we are going to clear
    // if we subtract low from high, we will receive the incomplete number of bits (e.g. high = 3 low = 3 => high - low = 0 (but we are going to replace 1 bit, so we need to add 1 to the subtraction))
    unsigned int width = high - low + 1;

    // next we are creating a mask
    // let's explain every part of it:
    // 1. ((1 << width) - 1) - we are creating a sequence of ones at the positions 0 to width - 1
    // it is basically our "essensial" part of the mask
    // i've tried to remove "- 1" from the equation, and it ended up breaking down the mask completely (without "- 1" the mask clears only the highest bit of the range, not the entirety of it)
    // "- 1" gives us "width" of ones instead of the one one that there is after (1 << width)

    // 2. ((1 << width) - 1) << low - by using << low we are moving (shifting) the set bits (the ones) to the target positions
    // the lowest bit of the ones ends up being at the position of low
    // the highest bit of the ones ends up being at the position of high
    // e.g. 0b00000111 << low, where low is 1 => 0b00001110
    // basically now we have a mask that keeps target bits (e.g. in range 3-1) as they are and zeros out every other bit in the number (if the mask is used with AND (&) with the value)

    // 3. NOT (~) - in my personal opinion, it is second most important thing in the formula (after ((1 << width) - 1))
    // it inverts the bits (e.g. ~0b00011110 = 0b11100001)
    // we need it to go from the mask that protects the target bits and removes bits around it to the mask that removes the target bits and keeps the other bits
    // because at the next step (AND with the value) we are going to see that not inverted mask isn't going to be useful at all
    uint32_t clear_mask = ~(((1 << width) - 1) << low);

    // first of all, we are dereferencing the value pointer because we want to manipulate on its value, not on its address
    // but back to the topic, we are using AND on the value and the clear mask because we know how the AND works - it gives 1 only when two inputs are 1, otherwise - 0.
    // so in pair with clear_mask, that has as unset bits the bits we want to clear the value from and other values as 1s, AND is basically the only option we have
    // as an example, *value = 0b00001110 (we want to clear bits 3-2), clear_mask = 0b11110011 (has bits 2-1 as 0)
    // 0b00001110 & 0b11110011 = 0b00000010 = 2 (we successfully cleared the bites)
    *value &= clear_mask;

    // the next step: replace the cleared bits with the new_bits
    // as i discovered (by using some value and making the function break on paper), new_bits could not be just some number
    // new_bits is a set of bits that are going to replace the range high-low in the value
    // new_bits should contain only the bits that are going to be inputted to the value, all other bits are zero
    // if the new_bits has some other 1s, they are going to break the logic by shifting to the new positions and corrupting the value

    // so we need to make a mask to the new_bits var as well to prevent this from happening and to allow to basically put different numbers as a new_bits

    // the mask needs to protect the selected range of bits (high - low) and set every other 1 as 0
    // this mask is the clear_mask without NOT (inverting) - we do not want to clear the bits in high - low, we want to protect them and clear bits outside the range
    uint32_t new_bits_mask = (((1 << width) - 1) << low);

    // we are protecting the target range and clearing the outside "noise"
    // for example, new_bits = 0b11111111, high = 5, low = 2; => width = 5 - 2 + 1 = 4 (bit 2, 3, 4, 5);
    // new_bits_mask = (((1 << 4) - 1) << 2) = (((0b00000001 << 4) - 1) << 2) = ((0b00010000 - 1) << 2) = (16 - 1) << 2 = 15 << 2 = 0b00001111 << 2 = 0b00111100 ("protected bits": 5-2)
    // high = 5, low = 2; new_bits = new_bits & 0b00111100 = 0b11111111 & 0b00111100 = 0b00111100 = 2^5 + 2^4 + 2^3 + 2^2 = 32 + 16 + 8 + 4 = 40 + 20 = 60
    new_bits &= new_bits_mask;

    // finally, replacing the bits
    // we are again dereferencing the value pointer to get what is "behind" the address
    // we are using OR (|) because we know that OR gives us 1 when at least one of the inputs is 1 (1 and 0, 0 and 1, 1 and 1)
    // since the new_bits is essensially the range of bits inside the "container of zeros" and we want to "fuse" *value with new_bits without losing ones in *value after the clearning up the target range, OR will help us to do so
    // for example, new_bits = 0b00111100, *value = 0b01000001 => *value = 0b01000001 | 0b00111100 = 0b01111101 = 2^6 + 2^5 + 2^4 + 2^3 + 2^2 + 2^0 = 64 + 32 + 16 + 8 + 4 + 1 = 80 + 40 + 5 = 125;
    *value |= new_bits; // or if we did not make "new_bits &= new_bits_mask": *value |= (new_bits & new_bits_mask) or *value = *value | (new_bits & new_bits_mask) or *value = *value | (new_bits & (((1 << width) - 1) << low)) or *value = *value | (new_bits & (((1 << (high - low + 1)) - 1) << low))
}

int main(void)
{
    uint32_t value = 25;
    unsigned int low = 1, high = 3;
    printf("extract bits %u-%u in number %u: %u\n", high, low, value, extract_bits(value, high, low)); // 4
}
