#ifndef STM32F411_H
#define STM32F411_H

#include <stdint.h>

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

typedef struct
{
    volatile uint32_t SR;    // 0x00
    volatile uint32_t DR;    // 0x04
    volatile uint32_t BRR;   // 0x08
    volatile uint32_t CR[3]; // 0x0C, 0x10, 0x14
    volatile uint32_t GTPR;  // 0x18
} USART_RegDef_t;

typedef struct
{
    volatile uint32_t LISR;   // 0x0000
    volatile uint32_t HISR;   // 0x0004
    volatile uint32_t LIFCR;  // 0x0008
    volatile uint32_t HIFCR;  // 0x000C
    volatile uint32_t S0CR;   // 0x0010
    volatile uint32_t S0NDTR; // 0x0014
    volatile uint32_t S0PAR;  // 0x0018
    volatile uint32_t S0M0AR; // 0x001C
    volatile uint32_t S0M1AR; // 0x0020
    volatile uint32_t S0FCR;  // 0x0024
    volatile uint32_t S1CR;   // 0x0028
    volatile uint32_t S1NDTR; // 0x002C
    volatile uint32_t S1PAR;  // 0x0030
    volatile uint32_t S1M0AR; // 0x0034
    volatile uint32_t S1M1AR; // 0x0038
    volatile uint32_t S1FCR;  // 0x003C
    volatile uint32_t S2CR;   // 0x0040
    volatile uint32_t S2NDTR; // 0x0044
    volatile uint32_t S2PAR;  // 0x0048
    volatile uint32_t S2M0AR; // 0x004C
    volatile uint32_t S2M1AR; // 0x0050
    volatile uint32_t S2FCR;  // 0x0054
    volatile uint32_t S3CR;   // 0x0058
    volatile uint32_t S3NDTR; // 0x005C
    volatile uint32_t S3PAR;  // 0x0060
    volatile uint32_t S3M0AR; // 0x0064
    volatile uint32_t S3M1AR; // 0x0068
    volatile uint32_t S3FCR;  // 0x006C
    volatile uint32_t S4CR;   // 0x0070
    volatile uint32_t S4NDTR; // 0x0074
    volatile uint32_t S4PAR;  // 0x0078
    volatile uint32_t S4M0AR; // 0x007C
    volatile uint32_t S4M1AR; // 0x0080
    volatile uint32_t S4FCR;  // 0x0084
    volatile uint32_t S5CR;   // 0x0088
    volatile uint32_t S5NDTR; // 0x008C
    volatile uint32_t S5PAR;  // 0x0090
    volatile uint32_t S5M0AR; // 0x0094
    volatile uint32_t S5M1AR; // 0x0098
    volatile uint32_t S5FCR;  // 0x009C
    volatile uint32_t S6CR;   // 0x00A0
    volatile uint32_t S6NDTR; // 0x00A4
    volatile uint32_t S6PAR;  // 0x00A8
    volatile uint32_t S6M0AR; // 0x00AC
    volatile uint32_t S6M1AR; // 0x00B0
    volatile uint32_t S6FCR;  // 0x00B4
    volatile uint32_t S7CR;   // 0x00B8
    volatile uint32_t S7NDTR; // 0x00BC
    volatile uint32_t S7PAR;  // 0x00C0
    volatile uint32_t S7M0AR; // 0x00C4
    volatile uint32_t S7M1AR; // 0x00C8
    volatile uint32_t S7FCR;  // 0x00CC
} DMA_RegDef_t;

typedef struct
{
    volatile uint32_t SR;    // 0x00
    volatile uint32_t CR1;   // 0x04
    volatile uint32_t CR2;   // 0x08
    volatile uint32_t SMPR1; // 0x0C
    volatile uint32_t SMPR2; // 0x10
    volatile uint32_t JOFR1; // 0x14
    volatile uint32_t JOFR2; // 0x18
    volatile uint32_t JOFR3; // 0x1C
    volatile uint32_t JOFR4; // 0x20
    volatile uint32_t HTR;   // 0x24
    volatile uint32_t LTR;   // 0x28
    volatile uint32_t SQR1;  // 0x2C
    volatile uint32_t SQR2;  // 0x30
    volatile uint32_t SQR3;  // 0x34
    volatile uint32_t JSQR;  // 0x38
    volatile uint32_t JDR1;  // 0x3C
    volatile uint32_t JDR2;  // 0x40
    volatile uint32_t JDR3;  // 0x44
    volatile uint32_t JDR4;  // 0x48
    volatile uint32_t DR;    // 0x4C
} ADC_RegDef_t;

typedef struct
{
    uint32_t RESERVED;     // 0x00
    volatile uint32_t CCR; // 0x04
} ADC_CommonRegisters_RegDef_t;

typedef struct
{
    ADC_RegDef_t ADC1;
    uint32_t RESERVED[172]; // 0x50 - 0x2FC -> 0x2B0 -> 688 bytes in total
    ADC_CommonRegisters_RegDef_t CommonRegisters;
} ADC_Global_RegDef_t;

// Alternate Function Modes
typedef enum
{
    GPIO_AF0 = 0, // 0 (0b0000)
    GPIO_AF1,     // 1 (0b0001)
    GPIO_AF2,     // 2 (0b0010)
    GPIO_AF3,     // 3 (0b0011)
    GPIO_AF4,     // 4 (0b0100)
    GPIO_AF5,     // 5 (0b0101)
    GPIO_AF6,     // 6 (0b0110)
    GPIO_AF7,     // 7 (0b0111)
    GPIO_AF8,     // 8 (0b1000)
    GPIO_AF9,     // 9 (0b1001)
    GPIO_AF10,    // 10 (0b1010)
    GPIO_AF11,    // 11 (0b1011)
    GPIO_AF12,    // 12 (0b1100)
    GPIO_AF13,    // 13 (0b1101)
    GPIO_AF14,    // 14 (0b1110)
    GPIO_AF15     // 15 (0b1111)

} GPIO_AF_t;

// ================== Base addresses ==================

#define RCC_BASE 0x40023800UL

#define TIM2_BASE 0x40000000UL
#define TIM3_BASE 0x40000400UL
#define TIM4_BASE 0x40000800UL
#define TIM5_BASE 0x40000C00UL

#define GPIOA_BASE 0x40020000UL
#define GPIOB_BASE 0x40020400UL
#define GPIOC_BASE 0x40020800UL
#define GPIOD_BASE 0x40020C00UL
#define GPIOE_BASE 0x40021000UL
#define GPIOH_BASE 0x40021C00UL

#define USART2_BASE 0x40004400UL

#define DMA1_BASE 0x40026000UL
#define DMA2_BASE 0x40026400UL

#define ADC1_BASE 0x40012000UL

#

// ================== Registers ==================

#define RCC ((RCC_RegDef_t *)RCC_BASE)

#define TIM2 ((TIM_RegDef_t *)TIM2_BASE)
#define TIM3 ((TIM_RegDef_t *)TIM3_BASE)
#define TIM4 ((TIM_RegDef_t *)TIM4_BASE)
#define TIM5 ((TIM_RegDef_t *)TIM5_BASE)

#define GPIOA ((GPIO_RegDef_t *)GPIOA_BASE)
#define GPIOB ((GPIO_RegDef_t *)GPIOB_BASE)
#define GPIOC ((GPIO_RegDef_t *)GPIOC_BASE)
#define GPIOD ((GPIO_RegDef_t *)GPIOD_BASE)
#define GPIOE ((GPIO_RegDef_t *)GPIOE_BASE)
#define GPIOH ((GPIO_RegDef_t *)GPIOH_BASE)

#define USART2 ((USART_RegDef_t *)USART2_BASE)

#define DMA1 ((DMA_RegDef_t *)DMA1_BASE)
#define DMA2 ((DMA_RegDef_t *)DMA2_BASE)

#define ADC ((ADC_Global_RegDef_t *)ADC1_BASE)
#define ADC1 ((ADC_RegDef_t *)ADC1_BASE)
#endif
