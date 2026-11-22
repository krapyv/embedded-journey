#include "FreeRTOS.h"
#include "log_event.h"
#include "app_config.h"
#include "uart.h"
#include "mcp2515.h"
#include "i2c.h"
#include "bmp280.h"
#include "queue.h"
#include "task.h"
#include <stdio.h>

// globally declared vaiable with physically allocated memory in RAM
I2C_HandleTypeDef hi2c;

volatile uint8_t can_int_flag = 0;
volatile uint8_t can_bus_off = 0;
volatile uint8_t can_intf_stuck = 0;

QueueHandle_t sensorQueue;
QueueHandle_t logQueue;

static uint32_t sensor_log_drop_counter = 0;

char *volatile vAppSOH_TaskName;

/* static logger queue */
// used to hold the queue's data structure
static StaticQueue_t xLoggerStaticQueue;

// the array to use as the queue's storage area
// at least queue_length * item_size -> logStorage = 6 * 24 = 144
static uint8_t ucLoggerQueueStorageArea[LOGGER_QUEUE_LENGTH * LOGGER_QUEUE_ITEM_SIZE];

/* sensor task */
// structure that will hold the TCB of the task
static StaticTask_t xSensorTaskBuffer;

// buffer that the task will use as its stack
static StackType_t xSensorStack[STACK_TASK_DEPTH];

/* logger task */
// structure that will hold the TCB of the task
static StaticTask_t xLoggerTaskBuffer;

// buffer that the task will use as its stack
static StackType_t xLoggerStack[STACK_TASK_DEPTH];

void vSensorTask(void *pvParameters)
{
    (void)pvParameters;
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
            sensor_log_drop_counter++;
        }

        if (xTaskDelayUntil(&xLastWakeTime, xPeriod) == pdFALSE)
        {
            // the task overran its period
            // for sampling tasks, skil missed periods, don't catch up.
            // todo: log it as its own event and resync - set xLastWakeTime to the current tick count
            // the missed periods are then skipped instead of replayed, and the schedule continues from now
        }
    }
}

void vLoggerTask(void *pvParameters)
{
    (void)pvParameters;
    for (;;)
    {
        LogEvent_t event;
        BaseType_t queue_receive_value = xQueueReceive(logQueue, &event, portMAX_DELAY);

        if (queue_receive_value == pdTRUE)
        {

            char buffer[128];
            int len;
            switch ((LogEvent_ID_e)event.event_id)
            {
            case EVT_SENSOR_READ_OK:
            {
                len = snprintf(buffer, sizeof(buffer), "Event ID = %u, temp = %ld, press = %lu, seq = %lu, tick = %lu\r\n", event.event_id, (int32_t)event.arg0, event.arg1, event.arg2, event.tick);

                // the line could be at most 95 characters (with \n and \r)
                // Event ID = 111, temp = 11111111111, press = 1111111111, seq = 1111111111, tick = 1111111111\r\n
                // so the clamp is a safety net
                break;
            }
            case EVT_CAN_TX_OK:
                len = snprintf(buffer, sizeof(buffer), "CAN TX ok, seq = %lu\r\n", event.arg0);

                break;

            case EVT_CAN_TX_ERROR:
                len = snprintf(buffer, sizeof(buffer), "CAN TX error, EFLG = 0x%02lX, seq = %lu\r\n", event.arg0, event.arg1);
                break;

            case EVT_SENSOR_READ_FAIL:
                len = snprintf(buffer, sizeof(buffer), "BMP280 read failed, err = %lu\r\n", event.arg0);
                break;

            case EVT_BUTTON_STATUS:
                len = snprintf(buffer, sizeof(buffer), "logQ wait = %lu, sensorTask drops = %lu, CanTxTask drops = %lu, UART TX drops = %lu\r\n", event.arg0, event.arg1, event.arg2, event.arg3);
                break;

            default:
                len = snprintf(buffer, sizeof(buffer), "INVALID EVENT ID: %u !\r\n", event.event_id);
                break;
            }

            if (len < 0)
            {
                len = snprintf(buffer, sizeof(buffer), "ERROR!\r\n");
            }
            if (len >= (int)sizeof(buffer))
            {
                len = sizeof(buffer) - 1; // 128 - 1
            }

            usart2_tx_write((const uint8_t *)buffer, (uint16_t)len);
        }
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                   char *pcTaskName)
{
    (void)xTask;

    taskDISABLE_INTERRUPTS();

    vAppSOH_TaskName = pcTaskName;

    for (;;)
    {
    }
}

int main(void)
{
    usart2_init();

    logQueue = xQueueCreateStatic(LOGGER_QUEUE_LENGTH, LOGGER_QUEUE_ITEM_SIZE, ucLoggerQueueStorageArea, &xLoggerStaticQueue);
    configASSERT(logQueue);

    // tasks creation
    TaskHandle_t xLoggerHandle = xTaskCreateStatic(vLoggerTask, "LOGGER_TASK", STACK_TASK_DEPTH, NULL, PRIORITY_LOGGER_TASK, xLoggerStack, &xLoggerTaskBuffer);
    configASSERT(xLoggerHandle);

    TaskHandle_t xSensorHandle = xTaskCreateStatic(vSensorTask, "SENSOR_TASK", STACK_TASK_DEPTH, NULL, PRIORITY_SENSOR_TASK, xSensorStack, &xSensorTaskBuffer);
    configASSERT(xSensorHandle);

    // start the FreeRTOS scheduler running
    vTaskStartScheduler();

    for (;;)
    {
    }
}