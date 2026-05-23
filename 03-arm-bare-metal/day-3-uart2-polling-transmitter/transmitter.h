#ifndef TRANSMITTER_H
#define TRANSMITTER_H

#include <stdint.h>

// base addresses
#define USART2_BASE 0x40004400
#define RCC_BASE 0x40023800
#define GPIOA_BASE 0x40020000

// RCC registers
#define RCC_AHB1ENR *(volatile uint32_t *)(RCC_BASE + 0x30)
#define RCC_APB1ENR *(volatile uint32_t *)(RCC_BASE + 0x40) // bit 17 enables USART2 clock

// GPIO registers
#define GPIOA_MODER *(volatile uint32_t *)(GPIOA_BASE + 0x00) // we need to set the bits of our pin to 10 (alternate mode)
// PA2: bits 8 - 11
// PA3: bits 12 - 15
#define GPIOA_AFRL *(volatile uint32_t *)(GPIOA_BASE + 0x20)

// USART2 registers
#define USART2_SR *(volatile uint32_t *)(USART2_BASE + 0x00) // we care only about bit 7 TXE, bit 6 TC and bit 5 RXNE
#define USART2_DR *(volatile uint32_t *)(USART2_BASE + 0x04)
#define USART2_BRR *(volatile uint32_t *)(USART2_BASE + 0x08)
#define USART2_CR1 *(volatile uint32_t *)(USART2_BASE + 0x0C)

// SysTick registers
#define SYST_CSR *(volatile uint32_t *)(0xE000E010)
#define SYST_RVR *(volatile uint32_t *)(0xE000E014)
#define SYST_CVR *(volatile uint32_t *)(0xE000E018)

#endif