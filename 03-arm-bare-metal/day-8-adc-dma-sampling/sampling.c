#include "core_cm4.h"
#include "stm32f411.h"
#include "sampling.h"

void clocks_init(void)
{
// ---- PORT CLOCK ENABLING

// since only ports A and B can be ADC
#if (TARGET_SAMP_PORT == HARDWARE_PORT_A)
    RCC->AHB1ENR |= (1 << 0);

#elif (TARGET_SAMP_PORT == HARDWARE_PORT_B)
    RCC->AHB1ENR |= (1 << 1);
#else
#error "Invalid target port assignment! Must use HARDWARE_PORT_A or HARDWARE_PORT_B"
#endif

    // ---- DMA ENABLING ----

    // DMA2 (bit 22) for reading ADC data
    // DMA1 (bit 21) for USART2 communication

    // 0x3 = 0b11
    RCC->AHB1ENR |= 0x3 << 21;

    // ---- ADC1 ENABLING ----
    RCC->APB2ENR |= 1U << 8;

    // ---- TIM2 ENABLING ----
    RCC->APB1ENR |= 1U << 0;

    // ---- UART2 ENABLING ----
    RCC->APB1ENR |= 1U << 17;

#if (TARGET_SAMP_PORT == HARDWARE_PORT_A)
// Port A has already been enabled
#elif (TARGET_SAMP_PORT == HARDWARE_PORT_B)
    /* Port A enabling for USART */
    RCC->AHB1ENR |= 1U << 0;
#endif
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