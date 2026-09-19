#include <stdio.h>
#include <string.h>
#include "uart.h"
#include "stack.h"

Stack_t stack;
uint32_t max_size = 10;
uint32_t buffer[10];

char line_assembly_buf[32];
uint8_t write_idx;
uint8_t is_overflow;
uint8_t is_tokenization_stage;
uint8_t is_tokenization_overflow;

char *tokens[16];
uint8_t token_count;

bool prev_was_space = true;

TOKEN_Classification_t classify_token(char *token)
{
}

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
            for (uint8_t i = 0; i < 32; i++)
            {
                if (line_assembly_buf[i] == '\0')
                {
                    break;
                }
                if (prev_was_space && line_assembly_buf[i] != ' ')
                {
                    if (token_count >= 16)
                    {
                        is_tokenization_overflow = true;
                        break;
                    }
                    else
                    {
                        tokens[token_count] = &line_assembly_buf[i];
                        token_count++;
                    }
                }
                prev_was_space = (line_assembly_buf[i] == ' ');
            }
            if (is_tokenization_overflow)
            {
                printf("Too many tokens (16 is maximum)!\n\r");
            }
            else
            {
                if (token_count == 0)
                {
                    printf("Empty input!\n\r");
                }
                else
                {

                    // no overflow, the tokens are valid
                    if (strcmp(tokens[0], "HELP") == 0)
                    {
                        printf("HELP Command: It is a trainee project\n\r");
                    }
                    else if (strcmp(tokens[0], "EVAL") == 0)
                    {
                                        }
                    else if (strcmp(tokens[0], "ECHO") == 0)
                    {
                        for (uint8_t i = 0; i < 32; i++)
                        {
                            printf(line_assembly_buf[i]);
                        }
                        printf("\n\r");
                    }
                    else
                    {
                        // unknown command
                        printf("Unknown command\n\r");
                    }
                }
            }

            // after the tokenization and dispatch are done
            is_tokenization_stage = 0;
            token_count = 0;
            prev_was_space = true;
            write_idx = 0;
            is_tokenization_overflow = false;
            break;
        }
    }
}