#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    TRAFFIC_RED,
    TRAFFIC_RED_YELLOW,
    TRAFFIC_GREEN,
    TRAFFIC_YELLOW
} TrafficState_t;

// to store all the relevant data in one place, we need to create structs
typedef struct
{
    int red;
    int yellow;
    int green;
} TrafficLightPins_t;

typedef struct
{
    uint32_t *port;
    TrafficState_t current_state;
    TrafficLightPins_t pins; // stores which pin numbers on the port control which LED, FIXED -> never change after initialization
    uint32_t time_in_state_ms;
    bool pedestrian_pending;
} TrafficLight_t;

void traffic_light_init(TrafficLight_t *traffic_light, uint32_t *port, TrafficLightPins_t pins);
void traffic_light_update(TrafficLight_t *traffic_light, bool pedestrian_request, uint32_t interval_ms);

#endif
