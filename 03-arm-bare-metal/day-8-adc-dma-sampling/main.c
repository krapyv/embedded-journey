#include "sampling.h"
#include "uart.h"

int main(void)
{
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
            if (dma_half_b_ready == 1U)
            {
                // overrun
                overrun_count++;
            }

            __DMB();
            dma_half_a_ready = 0U;

            // stream the data [0; N/2 - 1]
            // sizeof(char) = 1
            // a half of 1000 elements of uint16_t = 500 elements
            // sizeof(uint16_t) = 2 => 500 elements = 1000 bytes
            usart2_write_string((char *)&adc_raw_buffer[0], (ADC_BUFFER_SIZE / 2U) * sizeof(uint16_t));
        }

        if (dma_half_b_ready == 1U)
        {
            if (dma_half_a_ready == 1U)
            {
                // overrun
                overrun_count++;
            }

            __DMB();
            dma_half_b_ready = 0U;

            // stream the data [N/2; N - 1]
            usart2_write_string((char *)&adc_raw_buffer[ADC_BUFFER_SIZE / 2U], (ADC_BUFFER_SIZE / 2U) * sizeof(uint16_t));
        }
    }
    return 0;
}