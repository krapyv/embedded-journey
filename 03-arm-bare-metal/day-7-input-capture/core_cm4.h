#ifndef CORE_CM4_H
#define CORE_CM4_H

#include <stdint.h>

// offsets:
// ISER: 0x00 0x04 0x08 0x0C 0x10 0x14 0x18 0x1C
// ICER:
typedef struct
{
    // Interrupt Set-enable Registers
    volatile uint32_t ISER[8]; // 0xE000E100-0xE000E11C
    // Interrupt Clear-enable Registers
    volatile uint32_t ICER[8]; // 0XE000E180-0xE000E19C
    // Interrupt Set-pending Registers
    volatile uint32_t ISPR[8]; // 0XE000E200-0xE000E21C
    // Interrupt Clear-pending Registers
    volatile uint32_t ICPR[8]; // 0XE000E280-0xE000E29C
    // Interrupt Active Bit Registers
    volatile uint32_t IABR[8]; // 0xE000E300-0xE000E31C
    // Interrupt Priority Registers
    volatile uint8_t IPR[240]; // 0xE000E400-0xE000E4EF
    // Software Trigger Interrupt Register
    volatile uint32_t STIR; // 0xE000EF00
} NVIC_RegDef_t;

#define NVIC_BASE (0xE000E100UL)

#define NVIC ((NVIC_RegDef_t *)NVIC_BASE)

#endif