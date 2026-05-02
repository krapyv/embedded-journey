#ifndef SIMULATED_GPIO_H
#define SIMULATED_GPIO_H

#include <stdint.h>

// naming convention: {module}_{action}_{target}

// === Pin operations (single pin) ===

// Set pin HIGH (1)
void gpio_set_pin(uint32_t *port, unsigned int pin);

// Set pin LOW (0)
void gpio_clear_pin(uint32_t *port, unsigned int pin);

// Toggle pin (HIGH-LOW or LOW-HIGH)
void gpio_toggle_pin(uint32_t *port, unsigned int pin);

// Read pin state (returns 1 if HIGH, returns 0 if LOW)
int gpio_read_pin(uint32_t port, unsigned int pin);

// === Port operations (multiple pins) ===

// Write value to a range of pins (high to low)
void gpio_write_pins(uint32_t *port, unsigned int high, unsigned int low, uint32_t value);

// Read value from a range of pins
uint32_t gpio_read_pins(uint32_t port, unsigned int high, unsigned int low);

// === Whole port operations ===

// Write entire 32-bit value to port
void gpio_write_port(uint32_t *port, uint32_t value);

// Read entire 32-bit value from port
uint32_t gpio_read_port(uint32_t port);

#endif