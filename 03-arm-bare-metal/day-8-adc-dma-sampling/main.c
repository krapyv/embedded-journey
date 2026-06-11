#include "sampling.h"
#include "uart.h"

int main(void)
{
    clocks_init();
    gpio_init();
    tim_init();
    dma_init();
    nvic_init();
    adc_init();
    uart_init();
    pipeline_enabling();

    while (1)
    {
        if ((dma_half_a_ready == 1U) && (dma_half_b_ready == 1U))
        {
            overrun_count++;

            __DMB();
            dma_half_a_ready = 0U;
            dma_half_b_ready = 0U;

            // drop the corrupt data

            continue;
        }
        if (dma_half_a_ready == 1U)
        {
            // stream the data [0; N/2 - 1]
            if (usart2_stream_dma((uint16_t *)&adc_raw_buffer[0], ADC_BUFFER_SIZE / 2U) == 0U)
            {
                dma_half_a_ready = 2U; // 2U - in-flight
                __DMB();
            }
            else
            {
                // failure: drop frame
                dma_half_a_ready = 0U;
                // DMB to complete all memory accesses before this point before doing any memory accesses after this point
                __DMB();
            }
        }

        if (dma_half_b_ready == 1U)
        {
            // stream the data [N/2; N - 1]
            if (usart2_stream_dma((uint16_t *)&adc_raw_buffer[ADC_BUFFER_SIZE / 2U], ADC_BUFFER_SIZE / 2U) == 0U)
            {
                dma_half_b_ready = 2U; // 2U - in-flight
                __DMB();
            }
            else
            {
                dma_half_b_ready = 0U;
                __DMB();
            }
        }
    }
    return 0;
}