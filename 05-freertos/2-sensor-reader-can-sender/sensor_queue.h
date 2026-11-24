#ifndef SENSOR_QUEUE_H
#define SENSOR_QUEUE_H

#include <stdint.h>

typedef struct
{
    int32_t temp;
    uint32_t press;
    uint32_t seq;
    uint32_t tick;
} SensorQueue_t;

#endif // SENSOR_QUEUE_H