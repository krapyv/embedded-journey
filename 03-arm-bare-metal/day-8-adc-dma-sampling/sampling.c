#include "core_cm4.h"
#include "stm32f411.h"
#include "sampling.h"

void clocks_init(SAMP_HandleTypeDef *hsamp)
{
    // ---- PORT CLOCK ENABLING

    // since only ports A and B can be ADC
    if (hsamp->Port == GPIOA)
    {
        RCC->AHB1ENR |= (1 << 0);
    }
    else if (hsamp->Port == GPIOB)
    {
        RCC->AHB1ENR |= (1 << 1);
    }
    else
    {
        // incorrect port
        return;
    }
}

void gpio_init()
{
}

void tim_init()
{
}

void dma_init()
{
}

void nvic_init()
{
}

void adc_init()
{
}

void uart_init()
{
}

void pipeline_enabling()
{
}