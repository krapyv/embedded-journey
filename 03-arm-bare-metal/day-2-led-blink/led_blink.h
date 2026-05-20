#ifndef LED_BLINK_H
#define LED_BLINK_H

#include <stdint.h>

// to make an led blink, we are going to manipulate on GPIO pins of STM32 BlackPill (PA5)

// to make an led blink, we need:
// 1. enable clock
// 2. set direction (output)
// 3. send data (HIGH 3.3V or LOW 0V)

// addresses from Black Pill documentation

// base addresses
#define RCC_BASE 0x40023800
#define GPIOA_BASE 0x40020000
#define GPIOB_BASE 0x40020400

// registers addresses (base + offsets)

// macros to get the values at that addresses
#define RCC_AHB1ENR (*(volatile uint32_t *)(RCC_BASE + 0x30))

#define GPIOA_MODER (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_ODR (*(volatile uint32_t *)(GPIOA_BASE + 0x14))

#define GPIOB_MODER (*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_IDR (*(volatile uint32_t *)(GPIOB_BASE + 0x10))

#endif