#include <math.h>
#include "osal_time.h"
#include "FreeRTOS.h"
#include "task.h"

uint32_t osal_get_ms(void)
{
    TickType_t xTick = xTaskGetTickCount();
    uint32_t xMS = pdTICKS_TO_MS(xTick);

    return xMS;
}

void osal_delay_ms(uint32_t ms)
{
    const TickType_t xDelay = (ms == 0U) ? 0U : (TickType_t)((((uint64_t)ms * configTICK_RATE_HZ) + 999ULL) / 1000ULL) + 1U;
    vTaskDelay(xDelay);
}