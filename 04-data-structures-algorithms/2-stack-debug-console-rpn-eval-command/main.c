#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
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
    uint8_t str_length = strlen(token);

    if (str_length == 0)
    {
        return TOKEN_INVALID;
    }
    else if (str_length == 1)
    {

        char *endptr;
        strtol(token, &endptr, 10);

        // if the endptr pointer points to the null terminator, the token in a valid digit
        if (*endptr == '\0')
        {
            return TOKEN_OPERAND;
        }

        switch (*token)
        {
        case '+':
            return TOKEN_OP_ADD;
        case '-':
            return TOKEN_OP_SUB;
        case '*':
            return TOKEN_OP_MULT;
        case '/':
            return TOKEN_OP_DIV;
        }
    }
    else if (str_length >= 2)
    {
        char *endptr;
        // strtol also accepts a leading '+', outside original grammar, accepted as harmless
        strtol(token, &endptr, 10);

        if (*endptr == '\0')
        {
            return TOKEN_OPERAND;
        }
    }

    // fall through, invalid token (neither an operator nor a digit)
    return TOKEN_INVALID;
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
                if (popped_byte == '\r')
                {
                    if (is_overflow)
                    {
                        write_idx = 0;
                        is_overflow = 0;
                        printf("Line overflow!\n\r");
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
                        // unreachable with single-char tokens: 16 tokens tightly packed need
                        // 2 * 16 - 1 = 31 chars minimum, which equals the line buffer's own cap -
                        // LINE overflow (case 0, write_idx <= 30) always fires first. Kept as
                        // defined behavior per spec; would only trigger if LINE_BUF_SIZE grew
                        // relative to MAX_TOKENS.
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
                if (prev_was_space)
                {
                    line_assembly_buf[i] = '\0';
                }
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
                        uint8_t is_error = 0;
                        uint32_t operand_A, operand_B, result;
                        long val;

                        for (uint8_t i = 1; i < token_count; i++)
                        {
                            if (is_error)
                            {
                                break;
                            }

                            TOKEN_Classification_t token_type = classify_token(tokens[i]);

                            switch (token_type)
                            {
                            // relies on long == int32_t on this toolchain (ILP32); would need re-checking if ported
                            case TOKEN_OPERAND:
                                val = strtol(tokens[i], NULL, 10);

                                // checking for stack overflow
                                if (!Stack_push(&stack, val))
                                {
                                    is_error = 1;
                                    printf("Stack overflow!\n\r");
                                    break;
                                }
                                break;
                            case TOKEN_OP_ADD:
                                if (!Stack_pop(&stack, &operand_A) || !Stack_pop(&stack, &operand_B))
                                {
                                    is_error = 1;
                                    printf("Stack underflow!\n\r");
                                    break;
                                }

                                result = operand_B + operand_A;
                                if (!Stack_push(&stack, result))
                                {
                                    is_error = 1;
                                    printf("Stack overflow!\n\r");
                                    break;
                                }
                                break;
                            case TOKEN_OP_SUB:
                                if (!Stack_pop(&stack, &operand_A) || !Stack_pop(&stack, &operand_B))
                                {
                                    is_error = 1;
                                    printf("Stack underflow!\n\r");
                                    break;
                                }

                                result = operand_B - operand_A;
                                if (!Stack_push(&stack, result))
                                {
                                    is_error = 1;
                                    printf("Stack overflow!\n\r");
                                    break;
                                }
                                break;
                            case TOKEN_OP_MULT:
                                if (!Stack_pop(&stack, &operand_A) || !Stack_pop(&stack, &operand_B))
                                {
                                    is_error = 1;
                                    printf("Stack underflow!\n\r");
                                    break;
                                }

                                result = operand_B * operand_A;
                                if (!Stack_push(&stack, result))
                                {
                                    is_error = 1;
                                    printf("Stack overflow!\n\r");
                                    break;
                                }
                                break;
                            case TOKEN_OP_DIV:
                                if (!Stack_pop(&stack, &operand_A) || !Stack_pop(&stack, &operand_B))
                                {
                                    is_error = 1;
                                    printf("Stack underflow!\n\r");
                                    break;
                                }

                                if (operand_A == 0)
                                {
                                    is_error = 1;
                                    printf("Division by 0! Aborted!\n\r");
                                    break;
                                }

                                result = (int32_t)operand_B / (int32_t)operand_A;
                                if (!Stack_push(&stack, result))
                                {
                                    is_error = 1;
                                    printf("Stack overflow!\n\r");
                                    break;
                                }
                                break;
                            case TOKEN_INVALID:
                                is_error = 1;
                                printf("Invalid token detected!\n\r");
                                break;
                            }
                        }

                        if (!is_error)
                        {
                            if (stack.top != 0)
                            {
                                printf("Invalid equation!\n\r");
                            }
                            else
                            {
                                uint32_t final_res;
                                Stack_pop(&stack, &final_res);

                                printf("Result: %d\n\r", (int32_t)final_res);
                            }
                        }
                    }
                    else if (strcmp(tokens[0], "ECHO") == 0)
                    {
                        for (uint8_t i = 1; i < token_count; i++)
                        {
                            printf(tokens[i]);
                            if (i != token_count - 1)
                            {
                                printf(" ");
                            }
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
            Stack_clear(&stack);
            break;
        }
    }
}