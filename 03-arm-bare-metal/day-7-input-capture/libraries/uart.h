#ifndef UART_H
#define UART_H

#include <stdint.h>

void timer_init();
void usart2_init();
void usart2_write_char(char c);
void usart2_write_string(char *string);

#endif