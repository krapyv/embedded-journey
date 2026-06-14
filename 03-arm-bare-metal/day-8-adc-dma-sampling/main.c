#include "sampling.h"
#include "uart.h"
#include <stdio.h>

#define TX_BUFFER_SIZE 40

void fast_format_line(char *dest_buf, char buffer_char, uint16_t pot_val, uint32_t overruns)
{
    dest_buf[0] = '[';
    dest_buf[1] = 'B';
    dest_buf[2] = 'U';
    dest_buf[3] = 'F';
    dest_buf[4] = 'F';
    dest_buf[5] = ' ';
    dest_buf[6] = buffer_char; // 'A' or 'B'
    dest_buf[7] = ']';
    dest_buf[8] = ' ';
    dest_buf[9] = 'P';
    dest_buf[10] = 'o';
    dest_buf[11] = 't';
    dest_buf[12] = ':';
    dest_buf[13] = ' ';

    dest_buf[17] = (char)('0' + (pot_val % 10U));
    pot_val /= 10U;
    dest_buf[16] = (char)('0' + (pot_val % 10U));
    pot_val /= 10U;
    dest_buf[15] = (char)('0' + (pot_val % 10U));
    pot_val /= 10U;
    dest_buf[14] = (char)('0' + (pot_val % 10U));

    dest_buf[18] = ' ';
    dest_buf[19] = '|';
    dest_buf[20] = ' ';
    dest_buf[21] = 'O';
    dest_buf[22] = 'v';
    dest_buf[23] = 'r';
    dest_buf[24] = 'n';
    dest_buf[25] = 's';
    dest_buf[26] = ':';
    dest_buf[27] = ' ';

    // Extract 4 digits for Overruns (handles up to 9999 for tracking)
    dest_buf[31] = (char)('0' + (overruns % 10U));
    overruns /= 10U;
    dest_buf[30] = (char)('0' + (overruns % 10U));
    overruns /= 10U;
    dest_buf[29] = (char)('0' + (overruns % 10U));
    overruns /= 10U;
    dest_buf[28] = (char)('0' + (overruns % 10U));

    dest_buf[32] = ' ';
    dest_buf[33] = '\r';
    dest_buf[34] = '\n';
    dest_buf[35] = '\0'; // Null terminator
}

static char tx_buffer_a[TX_BUFFER_SIZE];
static char tx_buffer_b[TX_BUFFER_SIZE];

int main(void)
{
    clocks_init();
    gpio_init();
    dma_init();
    uart_init();
    adc_init();
    nvic_init();
    tim_init();
    pipeline_enabling();

    while (1)
    {

        if (dma_half_a_ready == 1U)
        {
            __disable_irq();
            dma_half_a_ready = 2U; // 2U - in-flight
            __DMB();
            __enable_irq();

            fast_format_line(tx_buffer_a, 'A', adc_raw_buffer[250], overrun_count);
            // stream the data [0; N/2 - 1]
            if (usart2_stream_dma((uint16_t *)tx_buffer_a, 35U) != 1U)
            {
                __disable_irq();

                // failure: drop frame
                dma_half_a_ready = 0U;
                // DMB to complete all memory accesses before this point before doing any memory accesses after this point
                __DMB();
                __enable_irq();
            }
        }

        if (dma_half_b_ready == 1U)
        {
            __disable_irq();
            dma_half_b_ready = 2U; // 2U - in-flight
            __DMB();
            __enable_irq();

            fast_format_line(tx_buffer_b, 'B', adc_raw_buffer[750], overrun_count);

            // stream the data [N/2; N - 1]
            if (usart2_stream_dma((uint16_t *)tx_buffer_b, 35U) != 1U)
            {
                __disable_irq();

                dma_half_b_ready = 0U;
                __DMB();
                __enable_irq();
            }
        }
    }
    return 0;
}