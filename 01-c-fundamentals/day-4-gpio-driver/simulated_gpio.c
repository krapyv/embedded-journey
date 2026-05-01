#include "bitwise.h"

void gpio_set_pin(uint32_t *port, unsigned int pin)
{
    set_bit(port, pin);
}

// Set pin LOW (0)
void gpio_clear_pin(uint32_t *port, unsigned int pin)
{
    clear_bit(port, pin);
}

// Toggle pin (HIGH-LOW or LOW-HIGH)
void gpio_toggle_pin(uint32_t *port, unsigned int pin)
{
    toggle_bit(port, pin);
}

// Read pin state (returns 1 if HIGH, returns 0 if LOW)
int gpio_read_pin(uint32_t port, unsigned int pin)
{
    return is_bit_set(port, pin);
}

// === Port operations (multiple pins) ===

// Write value to a range of pins (high to low)
void gpio_write_pins(uint32_t *port, unsigned int high, unsigned int low, uint32_t value)
{
    replace_bits(port, high, low, value);
}

// Read value from a range of pins
uint32_t gpio_read_pins(uint32_t port, unsigned int high, unsigned int low)
{
    return extract_bits(port, high, low);
}

// === Whole port operations ===

// Write entire 32-bit value to port

// correct from the mathematical perspective, but shifting 32-bit values by 32 positions is risky because of undefined behavior
// void gpio_write_port(uint32_t *port, uint32_t value)
// {
//     // we do not need to write a new function specifically for the 32-bit value replacement
//     // in our function
//     // unsigned int width = high - low + 1; <--- 31 - 0 + 1 = 32 bits are going to be replaced

//     // uint32_t clear_mask = ~(((1 << width) - 1) << low); <--- ~(((1 << 32) - 1) << 0) = ~(0 - 1) = ~(-1) = ~0b11111... (32 1s) = 0b0000 (32 0s) = 0;

//     // *value &= clear_mask;  <--- no matter what is inside *value, *value & 0 = 0b000... = 0

//     // uint32_t new_bits_mask = (((1 << width) - 1) << low); <---- (((1 << 32) - 1) << 0) = (-1) = 0b1111.... (32 1s) (the mask preserves all 32 bits of new_bits)

//     // *value |= (new_bits & new_bits_mask); <---- *value = 0 | (new_bits & (-1)) = new_bits <---- because new_bits after the masking is the new_bits. and 0 OR new_bits is new_bits (OR gives us 1 when at least one input is 1: 1 and 0, 0 and 1, 1 and 1. 0 and 0 gives 0)

//     // so all 32 bits replaced
//     replace_bits(port, 31, 0, value);
// }

// // Read entire 32-bit value from port
// uint32_t gpio_read_port(uint32_t port)
// {
//     // the same with the read function

//     // we are passing 31 and 0 as high and low
//     // unsigned int width = high - low + 1; <---- width = 31 - 0 + 1 = 32 <---- all bits are going to be read out
//     // uint32_t mask = (1 << width) - 1; <----- mask = (1 << 32) - 1 = (0 - 1) = -1 = 0b11111... (32 1s) <---- the mask keeps all the bits of the value in place, not changing anything
//     // return (value >> low) & mask; <---- (value >> low) & mask = (value >> 0) & (-1) = value & (-1) = value & 0b111111... (32 1s) = value <---- all bits are read
//     return extract_bits(port, 31, 0);
// }

// Write entire 32-bit value to port
void gpio_write_port(uint32_t *port, uint32_t value)
{
    *port = value; // replacing all the bits inside the port
}

// Read entire 32-bit value from port
uint32_t gpio_read_port(uint32_t port)
{
    return port; // returning all 32 bits of the port value
}