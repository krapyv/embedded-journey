#include <stdint.h>
#include "traffic_light.h"
#include "simulated_gpio.h"

#define RED_DURATION_MS 5000
#define RED_YELLOW_DURATION_MS 1000
#define YELLOW_DURATION_MS 2000
#define GREEN_DURATION_MS 7000
#define GREEN_MIN_DURATION_MS 4000

void traffic_light_green_to_yellow(TrafficLight_t *traffic_light)
{
    traffic_light->time_in_state_ms = 0;
    traffic_light->current_state = TRAFFIC_YELLOW;

    gpio_set_pin(traffic_light->port, traffic_light->pins.yellow);
    gpio_clear_pin(traffic_light->port, traffic_light->pins.green);
}

void traffic_light_init(TrafficLight_t *traffic_light, uint32_t *port, TrafficLightPins_t pins)
{
    // initializing the struct with default values

    traffic_light->port = port;

    traffic_light->current_state = TRAFFIC_RED;

    traffic_light->pins.red = pins.red;
    traffic_light->pins.green = pins.green;
    traffic_light->pins.yellow = pins.yellow;

    gpio_set_pin(traffic_light->port, traffic_light->pins.red);      // red ON
    gpio_clear_pin(traffic_light->port, traffic_light->pins.yellow); // yellow OFF
    gpio_clear_pin(traffic_light->port, traffic_light->pins.green);  // green OFF

    traffic_light->time_in_state_ms = 0;

    traffic_light->pedestrian_pending = 0;
}

// the traffic light works as next:
// red -> red + yellow -> green -> yellow -> red

// after the pedestrian request was granted, it still should go by the order:
// green -> yellow -> red
void traffic_light_update(TrafficLight_t *traffic_light, bool pedestrian_request, uint32_t interval_ms)
{
    // i think i should add the inteval to the struct time_in_state_ms outside the switch
    traffic_light->time_in_state_ms += interval_ms;

    if (pedestrian_request && traffic_light->current_state == TRAFFIC_GREEN)
    {
        traffic_light->pedestrian_pending = true;
    }

    switch (traffic_light->current_state)
    {
    case TRAFFIC_RED:
        if (traffic_light->time_in_state_ms >= RED_DURATION_MS)
        {
            traffic_light->time_in_state_ms = 0;
            traffic_light->current_state = TRAFFIC_RED_YELLOW;

            // we are setting the pin [yellow] to 1
            // so both the red and the yellow ones are 1
            gpio_set_pin(traffic_light->port, traffic_light->pins.yellow);
        }
        break;

    case TRAFFIC_RED_YELLOW:
        if (traffic_light->time_in_state_ms >= RED_YELLOW_DURATION_MS)
        {
            traffic_light->time_in_state_ms = 0;
            traffic_light->current_state = TRAFFIC_GREEN;

            // ERROR! The pins struct in the traffic_light is basially "const"
            // we cannot change it after the initialization
            // in order to change the lights, we need to set/unset bits on the port

            // traffic_light->pins.red = 0;
            // traffic_light->pins.yellow = 0;

            // setting the green to 1 (now the cars could go)
            // unsetting the red and yellow one (the pedestrians should wait)
            gpio_set_pin(traffic_light->port, traffic_light->pins.green);
            gpio_clear_pin(traffic_light->port, traffic_light->pins.red);
            gpio_clear_pin(traffic_light->port, traffic_light->pins.yellow);
        }
        break;

    case TRAFFIC_GREEN:
        if (traffic_light->time_in_state_ms >= GREEN_DURATION_MS)
        {
            traffic_light_green_to_yellow(traffic_light);
        }
        else if (traffic_light->pedestrian_pending && traffic_light->time_in_state_ms >= GREEN_MIN_DURATION_MS)
        {

            // well, i thought that after the pedestrian pushed the button, it should immediately turn to red xD
            // i am not living a reality, it does not work this way

            // traffic_light->current_state = TRAFFIC_RED;
            traffic_light_green_to_yellow(traffic_light);
            traffic_light->pedestrian_pending = false;
        }
        break;

    case TRAFFIC_YELLOW:
        if (traffic_light->time_in_state_ms >= YELLOW_DURATION_MS)
        {
            traffic_light->time_in_state_ms = 0;
            traffic_light->current_state = TRAFFIC_RED;
            gpio_set_pin(traffic_light->port, traffic_light->pins.red);
            gpio_clear_pin(traffic_light->port, traffic_light->pins.yellow);
        }
        break;
    }
}