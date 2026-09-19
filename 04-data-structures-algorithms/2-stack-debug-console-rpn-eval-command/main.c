#include <stdio.h>
#include "uart.h"
#include "stack.h"

Stack_t stack;
uint32_t max_size = 10;
uint32_t buffer[10];

char line_assembly_buf[32];
uint8_t write_idx;
uint8_t is_overflow;
uint8_t is_tokenization_stage;

char *tokens[16];

void main()
{
    Stack_init(&stack, buffer, max_size);
    usart2_init();

    printf("Please enter a command\n\r");

    uint8_t popped_byte;
    while (1)
    {
        switch (is_tokenization_stage)
        {
        case 0:
            if (usart2_rx_pop(&popped_byte))
            {
                if (popped_byte == '\n')
                {
                    if (is_overflow)
                    {
                        write_idx = 0;
                        is_overflow = 0;
                    }
                    else
                    {
                        line_assembly_buf[write_idx] = '\0';
                        is_tokenization_stage = 1;
                    }
                }
                else
                {
                    if (!is_overflow)
                    {
                        if (write_idx <= 30)
                        {
                            line_assembly_buf[write_idx] = popped_byte;
                            write_idx++;
                        }
                        else
                        {
                            is_overflow = 1;
                        }
                    }
                }
            }
            break;

        case 1:

            // after the tokenization and dispatch are done
            is_tokenization_stage = 0;
            write_idx = 0;
            break;
        }
    }
}