#ifndef LOG_EVENT_H
#define LOG_EVENT_H

#include <stdint.h>

typedef enum
{
    EVT_CAN_TX_OK = 0,
    EVT_CAN_TX_ERROR,
    EVT_SENSOR_READ_FAIL,
    EVT_BUTTON_STATUS,
} LogEvent_ID_e;

typedef struct
{
    uint32_t arg0;
    uint32_t arg1;
    uint32_t arg2;
    uint32_t arg3;
    uint32_t tick;
    uint8_t event_id;
} LogEvent_t;

#endif // LOG_EVENT_H