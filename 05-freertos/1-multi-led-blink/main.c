#include <memory.h>
#include <stdio.h>
#include "app_config.h"
#include "led.h"
#include "stm32f411.h"
#include "core_cm4.h"
#include "FreeRTOS.h"
#include "task.h"

static LED_HandleTypeDef ledHandle = {
    .pin = 13,
    .rcc_bit = 2,
    .moder_reg = &(GPIOC->MODER),
    .odr_reg = &(GPIOC->ODR),
    .rcc_clk_reg = &(RCC->AHB1ENR)};

void vToggleLed(void *pvParameters)
{
    const TickType_t xDelay = pdMS_TO_TICKS(500);
    LED_HandleTypeDef *pxLedHandler = (LED_HandleTypeDef *)pvParameters;

    for (;;)
    {
        led_on(pxLedHandler);
        vTaskDelay(xDelay);
        led_off(pxLedHandler);
        vTaskDelay(xDelay);
    }
}

void main(void)
{
    // 0b000 => clear value: 0b111 = 2^2 + 2^1 + 2^0 = 4 + 2 + 1 = 7 = 0x7
    SCB->AIRCR = ((0x5FA << 16U) | ~(0x7 << 8U)); // or it is enough to do: SCB->AIRCR = (0x5FA << 16U) since all other bits should be zero
    led_init(&ledHandle);

    TaskHandle_t xHandle = NULL;
    BaseType_t type = xTaskCreate(vToggleLed, "ToggleLED", 64, &ledHandle, 6, &xHandle);

    configASSERT(xHandle);
    if (type == pdTRUE)
    {
    }
    else
    {
        // pdFalse
    }
}