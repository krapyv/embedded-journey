#include "uart.h"
#include "ring_buffer.h"

volatile uint32_t ms_passed = 0; // our timer
RingBuffer_t rx_buffer;
uint8_t raw_rx_storage[16];

void SysTick_Handler()
{
    ms_passed++;
}

void USART2_IRQHandler()
{
    // first of all, check whether the interrupt has been caused by the receiver
    // for this, check bit 5 RXNE of USART2_SR (if 1 - Received data is ready to be read, 0 - data is not received)
    if (USART2_SR & (1 << 5))
    {
        // if the result > 0 (true), then the bit is 1
        uint8_t temp = USART2_DR;
        ring_buffer_push(&rx_buffer, temp);
    }
}

// busy-wait loop
// void delay(uint32_t ms)
// {
//     uint32_t start = ms_passed;

//     while ((ms_passed - start) < ms)
//         ;
// }

void timer_init()
{
    // set SYST_CSR bit 2 (CLKSOURCE) to 1 (it is a processor clock), bit 1 (TICKINT) to 1 (to enable interruptions), bit 0 (ENABLE) to 1 to enable the counter
    // instead of using three parts OR, we could use 0x7 (in binary 0b0111)
    SYST_RVR = 15999; // 1 ms on 16 MHz processor

    SYST_CVR = 0; // to clear anything that could be inside

    SYST_CSR |= 0x7; // there is << 0, so just 0x7
}

void usart2_init()
{
    // initialize the ring buffer
    ring_buffer_init(&rx_buffer, raw_rx_storage, 16);

    // first of all, enable RCC clocks for GPIOA (bit 0) and USART (bit 17)
    RCC_AHB1ENR |= 1; // keep all existing bits and set the bit 0 to 1

    RCC_APB1ENR |= (1 << 17); // keep all existing bits and set the bit 17 to 1

    // next, set the GPIOA_MODER to AF (10 or 0x2)
    // since we are using PA2 and PA3, we need to set bits 2-3 and 4-5

    // clear the existing bits 4-5 and 6-7 inside the MODER
    GPIOA_MODER &= ~((0x3 << 6) | (0x3 << 4)); // 0x3 = 0b00000011
    // in ((0x3 << 6) | (0x3 << 4)) we are going to have 0b.....11110000
    // after the inversion (~), the result will be 0b1111....00001111
    // after ANDing with GPIOA_MODER, its bits 4-5 and 6-7 are cleared (AND gives 1 only when two inputs are 1)

    // set bits 6-7 and 4-5 to desired value (10)
    GPIOA_MODER |= (0x2 << 6) | (0x2 << 4);

    // According to the Referrence Manual, for USART1/2 we are using AF7
    // set the bits 8-11 and 12-15 of GPIOA_AFRL to 0111 (AF7)

    // 0x7 = 0b0111 in binary

    // firstly, let's clear the existing bits
    GPIOA_AFRL &= ~((0x7 << 12) | (0x7 << 8));

    GPIOA_AFRL |= (0x7 << 12) | (0x7 << 8); // the bits are set

    // now we need to initialize USART registers

    // according to the formula USARTDIV = clock frequency / 8 * (2 - OVER8) * Baud Rate

    // for my 16 MHz processor clock: USARTDIV  = 8.6805 => Mantissa (integer part)  = 8 = 0x8
    // fraction = 0.6805 * 16 = 10.88 => rounded to 11 = 0xB
    // the resulting USART_BRR = 0x008B

    USART2_BRR = 0x008B;

    // enabling USART, RXNEIE, TE, RE
    USART2_CR1 |= (1 << 13) | (1 << 5) | (1 << 3) | (1 << 2);

    NVIC_ISER1 |= (1 << 6); // bit 6 is responsible for the interrupt position 38
}

void usart2_write_char(char c)
{
    while (!(USART2_SR & (1 << 7)))
    {
    }

    USART2_DR = c;
}

void usart2_write_string(char *string)
{
    while (*string)
    {
        usart2_write_char(*string++);
    }
}

int main(void)
{
    usart2_init();
    timer_init();

    char *string = "Hello, World!";

    uint32_t last_transmission = 0;

    while (1)
    {
        if (ms_passed - last_transmission >= 500)
        {
            usart2_write_string(string);
            last_transmission = ms_passed;
        }
    }

    return 0;
}