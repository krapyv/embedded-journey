#include <memory.h>
#include <stdio.h>
#include "app_config.h"
#include "led.h"
#include "uart.h"
#include "stm32f411.h"
#include "core_cm4.h"
#include "FreeRTOS.h"
#include "task.h"

static LED_HandleTypeDef ledHandlePC13 = {
    .pin = 13,
    .rcc_bit = 2,
    .moder_reg = &(GPIOC->MODER),
    .bsrr_reg = &(GPIOC->BSRR),
    .rcc_clk_reg = &(RCC->AHB1ENR),
    .half_period_ms = 500,
    .counter = 0};

static LED_HandleTypeDef ledHandlePA1 = {
    .pin = 1,
    .rcc_bit = 0,
    .moder_reg = &(GPIOA->MODER),
    .bsrr_reg = &(GPIOA->BSRR),
    .rcc_clk_reg = &(RCC->AHB1ENR),
    .half_period_ms = 1200,
    .counter = 0};

static LED_HandleTypeDef ledHandlePA10 = {
    .pin = 10,
    .rcc_bit = 0,
    .moder_reg = &(GPIOA->MODER),
    .bsrr_reg = &(GPIOA->BSRR),
    .rcc_clk_reg = &(RCC->AHB1ENR),
    .half_period_ms = 1700,
    .counter = 0};

static uint32_t logger_half_period_ms = 1000U;

void vToggleLed(void *pvParameters)
{
    LED_HandleTypeDef *pxLedHandler = (LED_HandleTypeDef *)pvParameters;
    const TickType_t xDelay = pdMS_TO_TICKS(pxLedHandler->half_period_ms);

    for (;;)
    {
        pxLedHandler->counter++;
        led_on_bsrr(pxLedHandler);
        vTaskDelay(xDelay);
        led_off_bsrr(pxLedHandler);
        vTaskDelay(xDelay);
    }
}

void vToggleLedHighest(void *pvParameters)
{
    LED_HandleTypeDef *pxLedHandler = (LED_HandleTypeDef *)pvParameters;
    const TickType_t xDelay = pdMS_TO_TICKS(pxLedHandler->half_period_ms);

    TickType_t xLastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        pxLedHandler->counter++;
        led_on_bsrr(pxLedHandler);
        xTaskDelayUntil(&xLastWakeTime, xDelay);
        led_off_bsrr(pxLedHandler);
        xTaskDelayUntil(&xLastWakeTime, xDelay);
    }
}

void vPrintLogger(void *pvParameters)
{
    TickType_t tick_counter = xTaskGetTickCount();
    uint32_t *half_period_ms = (uint32_t *)pvParameters;
    const TickType_t xDelay = pdMS_TO_TICKS(*half_period_ms);

    for (;;)
    {
        printf("%lu | PC13: %lu | PA10: %lu | PA1: %lu\n", xTaskGetTickCount(), ledHandlePC13.counter, ledHandlePA10.counter, ledHandlePA1.counter);
        xTaskDelayUntil(&tick_counter, xDelay);
    }
}

void main(void)
{
    SCB->AIRCR = (0x5FA << 16U);
    led_init(&ledHandlePC13);
    led_init(&ledHandlePA1);
    led_init(&ledHandlePA10);
    usart2_init();

    TaskHandle_t xHandlePC13 = NULL;
    xTaskCreate(vToggleLed, "ToggleLED_PC13", 512, &ledHandlePC13, 2, &xHandlePC13);
    configASSERT(xHandlePC13);

    TaskHandle_t xHandlePA1 = NULL;
    xTaskCreate(vToggleLed, "ToggleLED_PA1", 512, &ledHandlePA1, 3, &xHandlePA1);
    configASSERT(xHandlePA1);

    TaskHandle_t xHandlePA10 = NULL;
    xTaskCreate(vToggleLedHighest, "ToggleLED_PA10", 512, &ledHandlePA10, 4, &xHandlePA10);
    configASSERT(xHandlePA10);

    TaskHandle_t xHandleLogger = NULL;
    xTaskCreate(vPrintLogger, "Logger", 512, &logger_half_period_ms, 1, &xHandleLogger);
    configASSERT(xHandleLogger);

    vTaskStartScheduler();

    for (;;)
    {
    }
}