#include <stdio.h>
#include "uart.h"
#include "stack.h"

Stack_t stack;
uint32_t max_size = 10;
uint32_t buffer[10];

void main()
{
    Stack_init(&stack, buffer, max_size);
    usart2_init();

    print("Please enter a command (max 16 symbols)\n\r");

    while (1)
    {
        }
}