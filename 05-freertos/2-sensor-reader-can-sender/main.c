#include "FreeRTOS.h"
#include "app_config.h"
#include "uart.h"
#include "mcp2515.h"
#include "i2c.h"
#include "bmp280.h"
#include "uart.h"
#include "queue.h"
#include "task.h"

// globally declared vaiable with physically allocated memory in RAM
I2C_HandleTypeDef hi2c;

volatile uint8_t can_int_flag = 0;
volatile uint8_t can_bus_off = 0;
volatile uint8_t can_intf_stuck = 0;

QueueHandle_t sensorQueue;
QueueHandle_t logQueue;

static uint32_t sensor_drop_counter = 0;

void vSensorTask(void *pvParameters)
{
    (void *)pvParameters;
    TickType_t xLastWakeTime;
    const TickType_t xPeriod = pdMS_TO_TICKS(SENSOR_TASK_PERIOD_MS);

    xLastWakeTime = xTaskGetTickCount();

    uint32_t counter = 0;

    // the loop that defines the task behavior
    for (;;)
    {
        LogEvent_t entry = {.event_id = (uint8_t)EVT_SENSOR_READ_OK, .arg0 = 2300, .arg1 = 1024, .arg2 = counter++, .arg3 = 0, .tick = xTaskGetTickCount()};
        BaseType_t queueSendRes = xQueueSendToBack(logQueue, &entry, 0); // 0 - try once and return immediately

        if (queueSendRes == errQUEUE_FULL)
        {
            // queue was full and the item is dropped
            sensor_drop_counter++;
        }

        if (xTaskDelayUntil(&xLastWakeTime, xPeriod) == pdFALSE)
        {
            // the task overran its period
            // todo: log it as its own event
        }
    }
}

void vLoggerTask(void *pvParameters)
{
}

int main(void)
{
    logQueue = xQueueCreate(LOGGER_QUEUE_LENGTH, LOGGER_QUEUE_ITEM_SIZE);

    if (logQueue == NULL)
    {
        // the queue could not be created
    }
}