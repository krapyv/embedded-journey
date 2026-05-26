#include "uart.h"
#include "ring_buffer.h"
#include "led.h"

extern RingBuffer_t rx_buffer; // access the global storage from uart.c
extern volatile uint32_t ms_passed;

int main(void)
{
    // Enable GPIOA clock directly
    *(volatile uint32_t *)0x40023830 |= (1 << 0);

    // Manually force PA5 to General Purpose Output (Bits 11:10 = 01)
    *(volatile uint32_t *)0x40020000 &= ~(3 << 10); // Clear bits 10 and 11
    *(volatile uint32_t *)0x40020000 |= (1 << 10);  // Set bit 10 to 1

    // Manually drive PA5 HIGH (Turn LED ON fully)
    *(volatile uint32_t *)0x40020014 |= (1 << 5);

    while (1)
    {
        // DO ABSOLUTELY NOTHING HERE.
        // No USART, No Timers, No Interrupts, No Ring Buffers.
    }
}