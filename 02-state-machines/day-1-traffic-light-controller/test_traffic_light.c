#include <stdio.h>
#include "simulated_gpio.h"
#include "traffic_light.h"

// helper to print LED state
void print_led_state(TrafficLight_t *light)
{
    // need to dereference the light pointer because my gpio_read_pin expects a value, not a pointer (the arrow dereferences the port pointer inside the struct)
    printf("LEDs: R=%d Y=%d G=%d ",
           gpio_read_pin(*light->port, light->pins.red),
           gpio_read_pin(*light->port, light->pins.yellow),
           gpio_read_pin(*light->port, light->pins.green));
}

int main(void)
{
    uint32_t port = 0;
    TrafficLight_t light;
    TrafficLightPins_t pins = {
        .red = 0,
        .yellow = 1,
        .green = 2};

    // initialization of the traffic lights
    traffic_light_init(&light, &port, pins);

    printf("=== Traffic Lights Test ===\n");

    for (int i = 0; i < 6000; i++)
    {
        // we are simulation the pedestrian button press at 11 seconds
        bool pedestrian_signal = (i >= 1100 && i < 1101);

        traffic_light_update(&light, pedestrian_signal, 10);

        if (i % 100 == 0)
        {
            printf("Time: %d.%ds, State: %d\n", i / 100, (i * 10) % 1000, light.current_state);
            print_led_state(&light);
        }
    }

    return 0;
}