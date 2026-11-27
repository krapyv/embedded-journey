#include "FreeRTOS.h"
#include "log_event.h"
#include "sensor_reading.h"
#include "app_config.h"
#include "uart.h"
#include "mcp2515.h"
#include "i2c.h"
#include "bmp280.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include <stdio.h>

// globally declared vaiable with physically allocated memory in RAM
I2C_HandleTypeDef hi2c;
static BMP280_HandleTypeDef hbmp;

volatile uint8_t can_int_flag = 0;
volatile uint8_t can_bus_off = 0;
volatile uint8_t can_intf_stuck = 0;

static QueueHandle_t sensorQueue;
static QueueHandle_t logQueue;

// drop counters
static uint32_t canTxTask_drop_counter = 0;
static uint32_t sensor_log_drop_counter = 0;

char *volatile vAppSOH_TaskName;

/* static logger queue */
// used to hold the queue's data structure
static StaticQueue_t xLoggerStaticQueue;

// the array to use as the queue's storage area
// at least queue_length * item_size -> logStorage = 6 * 24 = 144
static uint8_t ucLoggerQueueStorageArea[LOGGER_QUEUE_LENGTH * LOGGER_QUEUE_ITEM_SIZE];

/* static sensor queue */
static StaticQueue_t xSensorStaticQueue;

// the array to use as the queue's storage area: queue_length * item_size -> 1 * 16 = 16
static uint8_t ucSensorQueueStorageArea[SENSOR_QUEUE_LENGTH * SENSOR_QUEUE_ITEM_SIZE];

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

/* can tx task */
static StaticTask_t xCanTxTaskBuffer;
static StackType_t xCanTxStack[STACK_TASK_DEPTH];

/* status task */
static StaticTask_t xStatusTaskBuffer;
static StackType_t xStatusStack[STACK_TASK_DEPTH];

// button semaphore
static SemaphoreHandle_t xButtonSemaphore;
static StaticSemaphore_t xSemaphoreBuffer;

volatile uint32_t exti0_counter = 0;

static LogEvent_t task_ConfigureLog(uint8_t event_id, uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t tick)
{
    LogEvent_t log = {.event_id = event_id,
                      .arg0 = arg0,
                      .arg1 = arg1,
                      .arg2 = arg2,
                      .arg3 = arg3,
                      .tick = tick};

    return log;
}

static void sendEntryToQueue(LogEvent_t *entry, uint32_t *counter)
{
    BaseType_t queueSendRes = xQueueSendToBack(logQueue, entry, 0); // 0 - try once and return immediately

    if ((queueSendRes == errQUEUE_FULL) && (counter != NULL))
    {
        // queue was full and the item is dropped
        (*counter)++;
    }
}

void vCanTxTask(void *pvParameters)
{
    (void)pvParameters;

    const TickType_t xAbortPeriod = pdMS_TO_TICKS(2);

    SensorReading_t current_reading;
    SensorReading_t prev_reading;
    bool frame_is_pending = false;
    uint8_t read_status = 0;
    LogEvent_t entry;

    bool isConfigurationFailed = false;
    bool isLinkDead = false;
    bool abort_requested = false;

    MCP_Configure_t configure_res = mcp2515_configure(CAN_CNF1, CAN_CNF2, CAN_CNF3);

    if (configure_res == MCP_CONFIGURE_ERROR)
    {
        // log it and keep logging each period, without touching the MCP2515
        isConfigurationFailed = true;
    }

    for (;;)
    {
        BaseType_t queue_receive_value = xQueueReceive(sensorQueue, &current_reading, portMAX_DELAY);

        if (queue_receive_value == pdTRUE)
        {

            if (isConfigurationFailed)
            {
                entry = task_ConfigureLog((uint8_t)EVT_CAN_CONFIG_FAIL, current_reading.seq, 0, 0, 0, xTaskGetTickCount());
                sendEntryToQueue(&entry, &canTxTask_drop_counter);
            }
            else
            {

                if (frame_is_pending)
                {
                    uint8_t read_status_counter = 0;
                    for (;;)
                    {
                        // read status: bit 0 - RX0IF, 1 - RX1IF, 2 - TXREQ TXB0, 3 - TX0IF, 4 - TXREQ TXB1, 5 - TX1IF, 6 - TXREQ TXB2, 7 - TX2IF
                        mcp2515_read_status(&read_status, 1U);
                        read_status_counter++;

                        // try max 5 times
                        // if no, the SPI link died and something else has happened
                        if (read_status_counter < 5)
                        {

                            // isolate the TXREQ TXB0
                            // if TXREQ TXB0 is set - abort, wait <= 2ms (blocking), log TX_ERROR with EFLG and the previous seq
                            if (read_status & (1U << 2U))
                            {
                                // abort the frame means clear TXREQ
                                // TXBnCTRL: TXREQ bit 3
                                // mask: 0000 1000 = 2^3 = 0x8 - only the bit 3 changes
                                // data byte: 0000 0000 = 0x0 - the bit 3 changes to 0
                                mcp2515_bit_modify(TXB0CTRL, 0x8, 0x0);

                                // blocking
                                vTaskDelay(xAbortPeriod);

                                uint8_t eflg_val = 0;

                                mcp2515_read(EFLG, &eflg_val, 1U);

                                if (!abort_requested)
                                {
                                    entry = task_ConfigureLog((uint8_t)EVT_CAN_TX_ERROR, eflg_val, prev_reading.seq, 0, 0, xTaskGetTickCount());
                                    sendEntryToQueue(&entry, &canTxTask_drop_counter);
                                }

                                abort_requested = true;
                                // if the TXREQ is still set, then block for another 2 ms and read the status again

                                vTaskDelay(xAbortPeriod);
                            }
                            else
                            {
                                // the abort_requested != 1 - no abort was needed, so the previous frame was sent
                                if (!abort_requested)
                                {

                                    // tick - when the success was confirmed
                                    entry = task_ConfigureLog((uint8_t)EVT_CAN_TX_OK, prev_reading.seq, (uint32_t)prev_reading.temp, prev_reading.press, 0, xTaskGetTickCount());
                                    sendEntryToQueue(&entry, &canTxTask_drop_counter);
                                }
                                abort_requested = false;
                                // if TXREQ of TXB0 is not set, break the loop
                                break;
                            }
                        }
                        else
                        {
                            isLinkDead = true;
                            abort_requested = false;
                            break;
                        }
                    }

                    frame_is_pending = false;
                }

                if (!isLinkDead)
                {

                    // pack a new frame
                    uint8_t DLC = 0x8;
                    MCP_RTS_locations_t location = MCP_RTS_TXB0;

                    uint16_t id = 0x100; // 11 bits; 11-0, 15-12 are unused
                    uint8_t id_sidh = id >> 3;
                    uint8_t id_sidl = (id & 0x7) << 5;

                    /* settings for both press and temp */

                    // 1. Temperature
                    // since the temp is int32_t and we want only int16 => it consists of 2 int8_t bytes
                    // reconstruction of the int16_t => (temp_payload[0] << 8) | temp_payload[1]

                    int16_t narrowed_temp = (int16_t)current_reading.temp;

                    uint8_t temp_part_15_8 = (uint8_t)(narrowed_temp >> 8); // 15 14 13 12 11 10 9 8 - 8 bits
                    uint8_t temp_part_7_0 = (uint8_t)(narrowed_temp >> 0);  // 7 6 5 4 3 2 1 - 8 bits

                    // since the pressure is uint32_t => it consists of 4 uint8_t bytes

                    uint8_t press_part_31_24 = (uint8_t)(current_reading.press >> 24); // 31 30 29 28 27 26 25 24 - 8 bits
                    uint8_t press_part_23_16 = (uint8_t)(current_reading.press >> 16); // 23 22 21 20 19 18 17 16 - 8 bits
                    uint8_t press_part_15_8 = (uint8_t)(current_reading.press >> 8);   // 15 14 13 12 11 10 9 8 - 8 bits
                    uint8_t press_part_7_0 = (uint8_t)(current_reading.press >> 0);    // 7 6 5 4 3 2 1 - 8 bits

                    // SIDH, SIDL, EID8 (zeroed out, don't care), EID0 (zeroed out, don't care), DLC, 8 data bytes

                    uint8_t can_tx_payload[13] = {id_sidh, id_sidl, 0x00, 0x00, DLC, temp_part_15_8, temp_part_7_0, press_part_31_24, press_part_23_16, press_part_15_8, press_part_7_0, (uint8_t)current_reading.seq, 0x0};

                    mcp2515_load_tx_buffer(MCP_Load_TXB0SIDH, can_tx_payload, 13U);

                    mcp2515_rts(&location, 1U);

                    // remember the reading as previous and set pending
                    prev_reading = current_reading;
                    frame_is_pending = true;
                }
                else
                {
                    entry = task_ConfigureLog((uint8_t)EVT_CAN_TX_DEAD, current_reading.seq, 0, 0, 0, xTaskGetTickCount());
                    sendEntryToQueue(&entry, &canTxTask_drop_counter);
                }
            }
        }
    }
}

void vSensorTask(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastMainWakeTime, xLastSubWakeTime;
    const TickType_t xMainPeriod = pdMS_TO_TICKS(SENSOR_TASK_PERIOD_MS);
    const TickType_t xSubPeriod = pdMS_TO_TICKS(2);

    BMP280_ProcessState_t process_res;

    LogEvent_t entry;

    uint8_t isInitFailed = 0;

    // init loop
    xLastSubWakeTime = xTaskGetTickCount();

    for (;;)
    {
        I2C_Process();
        process_res = BMP280_Process(&hbmp);

        if ((process_res == BMP280_PROCESS_IDLE) || (process_res == BMP280_PROCESS_FAILED))
        {
            break;
        }
        xTaskDelayUntil(&xLastSubWakeTime, xSubPeriod);
    }

    if (process_res == BMP280_PROCESS_FAILED)
    {
        isInitFailed = 1;
    }

    xLastMainWakeTime = xTaskGetTickCount();

    uint32_t counter = 0;

    // the loop that defines the task behavior
    for (;;)
    {
        process_res = BMP280_PROCESS_FAILED;
        if (!isInitFailed)
        {

            BMP280_Status_t startMeasurement_res = BMP280_StartMeasurement(&hbmp);

            if (startMeasurement_res == BMP280_OK)

            {
                xLastSubWakeTime = xTaskGetTickCount();
                for (;;)
                {
                    I2C_Process();
                    process_res = BMP280_Process(&hbmp);

                    if ((process_res == BMP280_PROCESS_READY) || (process_res == BMP280_PROCESS_FAILED))
                    {
                        break;
                    }

                    xTaskDelayUntil(&xLastSubWakeTime, xSubPeriod);
                }
            }
        }

        if (process_res == BMP280_PROCESS_FAILED)
        {
            entry = task_ConfigureLog((uint8_t)EVT_SENSOR_READ_FAIL, (uint32_t)hbmp.last_error_code, 0, 0, 0, xTaskGetTickCount());
            sendEntryToQueue(&entry, &sensor_log_drop_counter);
        }
        else
        {
            SensorReading_t item = {
                .temp = hbmp.temp_value,
                .press = hbmp.press_value,
                .seq = counter,
                .tick = xTaskGetTickCount()};

            xQueueOverwrite(sensorQueue, &item);
            counter++;
        }

        if (xTaskDelayUntil(&xLastMainWakeTime, xMainPeriod) == pdFALSE)
        {
            // resynchronization
            // xLastMainWakeTime set to the current tick count
            // the missed periods are then skipped instead of replayed, and the schedule continues from now
            xLastMainWakeTime = xTaskGetTickCount();
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
            case EVT_CAN_TX_OK:
                len = snprintf(buffer, sizeof(buffer), "CAN TX ok, seq = %lu, temp = %ld, press = %lu\r\n", event.arg0, (int32_t)event.arg1, event.arg2);
                break;

            case EVT_CAN_TX_ERROR:
                len = snprintf(buffer, sizeof(buffer), "CAN TX error, EFLG = 0x%02lX, seq = %lu\r\n", event.arg0, event.arg1);
                break;

            case EVT_CAN_CONFIG_FAIL:
                len = snprintf(buffer, sizeof(buffer), "CAN TX config fail, seq = %lu\r\n", event.arg0);
                break;

            case EVT_CAN_TX_DEAD:
                len = snprintf(buffer, sizeof(buffer), "CAN TX dead, seq = %lu\r\n", event.arg0);
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

void vStatusTask(void *pvParameters)
{
    (void)pvParameters;
    LogEvent_t entry;

    for (;;)
    {
        // the task waits for a button signal
        if (xSemaphoreTake(xButtonSemaphore, portMAX_DELAY) == pdTRUE)
        {
            vTaskDelay(20000);
            entry = task_ConfigureLog((uint8_t)EVT_BUTTON_STATUS, uxQueueMessagesWaiting(logQueue), sensor_log_drop_counter, canTxTask_drop_counter, usart2_get_drop_counter(), xTaskGetTickCount());

            sendEntryToQueue(&entry, NULL);

            // for signalling, one side only gives (the ISR), the other side only takes (the task)
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

void EXTI0_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // clear the pending bit in EXTI_PR
    EXTI->PR = (1UL << 0U);

    exti0_counter++;

    xSemaphoreGiveFromISR(xButtonSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void button_init()
{
    // enable the GPIOA clock
    RCC->AHB1ENR |= (1UL << 0U);

    // enable the SYSCFG clock
    RCC->APB2ENR |= (1UL << 14U);

    // configure the pin PA0
    // set the input mode, MODER bits 0:1 to 00
    // 0b11 = 0x3
    GPIOA->MODER &= ~(0x3UL << 0U);

    // clear the PUPDR bits 0:1 since we are using external resistor
    GPIOA->PUPDR &= ~(0x3UL << 0U);

    // select the PA0 as the source input for the EXTI line 0 external interrupt
    // 0b1111 = 2^3 + 2^2 + 2^1 + 2^0 = 8 + 4 + 2 + 1 = 15 = 0xF
    SYSCFG->EXTICR1 &= ~(0xFUL << 0U);

    // enable falling trigger event for EXTI0
    EXTI->FTSR |= (1UL << 0U);

    // explicitly clear rising trigger event for EXTI0
    EXTI->RTSR &= ~(1UL << 0U);

    // NVIC: set the priority of 5
    // EXTI0 has the position 6 in the vector table
    NVIC->IPR[6] = (BUTTON_IRQ_PRIORITY << 4U);

    // unmask the EXTI0 line
    EXTI->IMR |= (1UL << 0U);

    // clear the pending bit before enabling ISER
    // PR is write-1-to-clear register
    EXTI->PR = (1UL << 0U);

    // enable the NVIC for the EXTI0
    NVIC->ISER[0] = (1UL << 6U);
}

int main(void)
{
    usart2_init();

    // BMP280

    // define and initialize structs
    hi2c.channel = I2C_CHANNEL_1;
    hi2c.scl_port = GPIOB;
    hi2c.scl_pin = 6;
    hi2c.sda_port = GPIOB;
    hi2c.sda_pin = 7;
    hi2c.state = I2C_STATE_IDLE;
    hi2c.sb_hits = 0;
    hi2c.stop_hits = 0;
    hi2c.start_pending_hits = 0;

    I2C_Init();
    spi_init(SPI_BR_8);

    BMP280_Ctrl_Meas_t meas = {
        .osrs_p = BMP280_OSRS_P_OVRSMP_1,
        .osrs_t = BMP280_OSRS_T_OVRSMP_1,
        .mode = BMP280_FORCED_MODE};

    BMP280_Status_t configure_res = BMP280_Configure(&hbmp, &hi2c, BMP280_I2C_ADDR, meas);

    configASSERT(configure_res == BMP280_OK);

    logQueue = xQueueCreateStatic(LOGGER_QUEUE_LENGTH, LOGGER_QUEUE_ITEM_SIZE, ucLoggerQueueStorageArea, &xLoggerStaticQueue);
    configASSERT(logQueue);

    sensorQueue = xQueueCreateStatic(SENSOR_QUEUE_LENGTH, SENSOR_QUEUE_ITEM_SIZE, ucSensorQueueStorageArea, &xSensorStaticQueue);
    configASSERT(sensorQueue);

    // tasks creation
    TaskHandle_t xLoggerHandle = xTaskCreateStatic(vLoggerTask, "LOGGER_TASK", STACK_TASK_DEPTH, NULL, PRIORITY_LOGGER_TASK, xLoggerStack, &xLoggerTaskBuffer);
    configASSERT(xLoggerHandle);

    TaskHandle_t xSensorHandle = xTaskCreateStatic(vSensorTask, "SENSOR_TASK", STACK_TASK_DEPTH, NULL, PRIORITY_SENSOR_TASK, xSensorStack, &xSensorTaskBuffer);
    configASSERT(xSensorHandle);

    TaskHandle_t xCanTxHandle = xTaskCreateStatic(vCanTxTask, "CAN_TX_TASK", STACK_TASK_DEPTH, NULL, PRIORITY_CAN_TX_TASK, xCanTxStack, &xCanTxTaskBuffer);
    configASSERT(xCanTxHandle);

    TaskHandle_t xStatusHandle = xTaskCreateStatic(vStatusTask, "STATUS_TASK", STACK_TASK_DEPTH, NULL, PRIORITY_STATUS_TASK, xStatusStack, &xStatusTaskBuffer);
    configASSERT(xStatusHandle);

    // before calling button_init() create the semaphore
    xButtonSemaphore = xSemaphoreCreateBinaryStatic(&xSemaphoreBuffer);
    configASSERT(xButtonSemaphore);
    button_init();

    // start the FreeRTOS scheduler running
    vTaskStartScheduler();

    for (;;)
    {
    }
}