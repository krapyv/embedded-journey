#include <stdio.h>
#include "simulated_gpio.h"

void print_port(uint32_t port, const char *label)
{
    printf("%-35s: 0x%08X\n", label, port);
}

int main(void)
{
    // single pin
    uint32_t my_port = 0;

    print_port(my_port, "Initial");

    gpio_set_pin(&my_port, 5);
    print_port(my_port, "gpio_set_pin(&port, 5)");

    gpio_clear_pin(&my_port, 5);
    print_port(my_port, "gpio_clear_pin(&port, 5)");

    gpio_toggle_pin(&my_port, 3);
    print_port(my_port, "gpio_toggle_pin(&port, 3)");

    int state = gpio_read_pin(my_port, 3);
    printf("gpio_read_pin(port, 3): %d\n", state);

    gpio_write_pins(&my_port, 7, 4, 0b1010);
    print_port(my_port, "gpio_write_pins(&port, 7, 4, 0b1010)");

    uint32_t pins = gpio_read_pins(my_port, 7, 4);
    printf("gpio_read_pins(port, 7, 4): 0x%08X\n", pins);

    gpio_write_port(&my_port, 0x12345678);
    print_port(my_port, "gpio_write_port(&port, 0x12345678)");

    uint32_t whole = gpio_read_port(my_port);

    printf("gpio_read_port(port): 0x%X\n", whole);

    return 0;
}