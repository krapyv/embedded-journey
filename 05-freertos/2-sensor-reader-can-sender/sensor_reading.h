#ifndef SENSOR_READING_H
#define SENSOR_READING_H

#include <stdint.h>

typedef struct
{
    int32_t temp;
    uint32_t press;
    uint32_t seq;
    uint32_t tick;
} SensorReading_t;

#endif // SENSOR_READING_H