#ifndef STM32F411_H
#define STM32F411_H

#include <stdint.h>

#define RCC_BASE 0x40023800UL
#define TIM2_BASE 0x40000000UL
#define GPIOA_BASE 0x40020000UL
#define GPIOB_BASE 0x40020400UL

typedef struct
{
    volatile uint32_t CR;           // 0x00
    volatile uint32_t PLLCFGR;      // 0x04
    volatile uint32_t CFGR;         // 0x08
    volatile uint32_t CIR;          // 0x0c
    volatile uint32_t AHB1RSTR;     // 0x10
    volatile uint32_t AHB2RSTR;     // 0x14
    volatile uint32_t RESERVED0[2]; // 0x18, 0x1C
    volatile uint32_t APB1RSTR;     // 0x20
    volatile uint32_t APB2RSTR;     // 0x24
    volatile uint32_t RESERVED1[2]; // 0x28, 0x2C
    volatile uint32_t AHB1ENR;      // 0x30
    volatile uint32_t AHB2ENR;      // 0x34
    volatile uint32_t RESERVED2[2]; // 0x38, 0x3C
    volatile uint32_t APB1ENR;      // 0x40
    volatile uint32_t APB2ENR;      // 0x44
    volatile uint32_t RESERVED3[2]; //  0x48, 0x4C
    volatile uint32_t AHB1LPENR;    // 0x50
    volatile uint32_t AHB2LPENR;    // 0x54
    volatile uint32_t RESERVED4[2]; // 0x58, 0x5C
    volatile uint32_t APB1LPENR;    // 0x60
    volatile uint32_t APB2LPENR;    // 0x64
    volatile uint32_t RESERVED5[2]; // 0x68, 0x6C
    volatile uint32_t BDCR;         // 0x70
    volatile uint32_t CSR;          // 0x74
    volatile uint32_t RESERVED6[2]; //  0x78, 0x7C
    volatile uint32_t SSCGR;        // 0x80
    volatile uint32_t PLLI2SCFGR;   // 0x84
    volatile uint32_t RESERVED7;    // 0x88
    volatile uint32_t DCKCFGR;      // 0x8C

} RCC_RegDef_t;

typedef struct
{
    volatile uint32_t CR1;       // 0x00
    volatile uint32_t CR2;       // 0x04
    volatile uint32_t SMCR;      // 0x08
    volatile uint32_t DIER;      // 0x0C
    volatile uint32_t SR;        // 0x10
    volatile uint32_t EGR;       // 0x14
    volatile uint32_t CCMR1;     // 0x18
    volatile uint32_t CCMR2;     // 0x1C
    volatile uint32_t CCER;      // 0x20
    volatile uint32_t CNT;       // 0x24
    volatile uint32_t PSC;       // 0x28
    volatile uint32_t ARR;       // 0x2C
    volatile uint32_t RESERVED0; // 0x30
    volatile uint32_t CCR[4];    // 0x34,0x38, 0x3C, 0x40
    volatile uint32_t RESERVED1; // 0x44
    volatile uint32_t DCR;       // 0x48
    volatile uint32_t DMAR;      // 0x4C
    volatile uint32_t OR;        // 0x50 (only TIM2 and TIM5)
} TIM_RegDef_t;

typedef struct
{
    volatile uint32_t MODER;   // 0x00
    volatile uint32_t OTYPER;  // 0x04
    volatile uint32_t OSPEEDR; // 0x08
    volatile uint32_t PUPDR;   // 0x0C
    volatile uint32_t IDR;     // 0x10
    volatile uint32_t ODR;     // 0x14
    volatile uint32_t BSRR;    // 0x18
    volatile uint32_t LCKR;    // 0x1C
    volatile uint32_t AFRL;    // 0x20
    volatile uint32_t AFRH;    // 0x24
} GPIO_RegDef_t;

#define RCC ((RCC_RegDef_t *)RCC_BASE)
#define TIM2 ((TIM_RegDef_t *)TIM2_BASE)
#define GPIOA ((GPIO_RegDef_t *)GPIOA_BASE)
#define GPIOB ((GPIO_RegDef_t *)GPIOB_BASE)

#endif
