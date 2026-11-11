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
    .bsrr_reg = &(GPIOC->BSRR),
    .rcc_clk_reg = &(RCC->AHB1ENR)};

void vToggleLed(void *pvParameters)
{
    const TickType_t xDelay = pdMS_TO_TICKS(500);
    LED_HandleTypeDef *pxLedHandler = (LED_HandleTypeDef *)pvParameters;

    for (;;)
    {
        led_on_bsrr(pxLedHandler);
        vTaskDelay(xDelay);
        led_off_bsrr(pxLedHandler);
        vTaskDelay(xDelay);
    }
}

void main(void)
{
    SCB->AIRCR = (0x5FA << 16U);
    led_init(&ledHandle);

    TaskHandle_t xHandle = NULL;
    xTaskCreate(vToggleLed, "ToggleLED", 512, &ledHandle, 2, &xHandle);
    configASSERT(xHandle);

    vTaskStartScheduler();

    for (;;)
    {
    }
}