#include "stdio.h"
#include "uart.h"

extern volatile uint32_t tx_hit_counter;

int main()
{
    usart2_init();
    usart2_dma_tx_init();

    while (1)
    {
        tx_hit_counter;
    }
    return 0;
}