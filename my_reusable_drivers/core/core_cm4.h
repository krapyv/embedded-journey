#ifndef CORE_CM4_H
#define CORE_CM4_H

#include <stdint.h>

// offsets:
// ISER: 0x000 - 0x01C
// Reserved: 0x80-0x20 = 0x60 = 96 bytes = 24 words
// ICER: 0x080 - 0x9C
// Reserved: 0x100-0x0A0 = 0x60 = 96 bytes = 24 words
// ISPR: 0x100 - 0x11C
// Reserved: 0x180-0x120 = 0x60 = 96 bytes = 24 words
// ICPR: 0x180 - 0x19C
// Reserved: 0x200-0x1A0 = 0x60 = 96 bytes = 24 words
// IABR: 0x200 - 0x21C
// Reserved: 0x300-0x220 = 0xE0 = 224 bytes = 56 words
// IPR: 0x300 - 0x3EF
// Reserved: 0xE00-0x3F0 = 0xA10 = 2576 bytes = 644 words
// SVIR: 0xE00
typedef struct
{
    // Interrupt Set-enable Registers
    volatile uint32_t ISER[8]; // 0xE000E100-0xE000E11C
    uint32_t RESERVED0[24];
    // Interrupt Clear-enable Registers
    volatile uint32_t ICER[8]; // 0XE000E180-0xE000E19C
    uint32_t RESERVED1[24];
    // Interrupt Set-pending Registers
    volatile uint32_t ISPR[8]; // 0XE000E200-0xE000E21C
    uint32_t RESERVED2[24];
    // Interrupt Clear-pending Registers
    volatile uint32_t ICPR[8]; // 0XE000E280-0xE000E29C
    uint32_t RESERVED3[24];
    // Interrupt Active Bit Registers
    volatile uint32_t IABR[8]; // 0xE000E300-0xE000E31C
    // Interrupt Priority Registers
    uint32_t RESERVED4[56];
    volatile uint8_t IPR[240]; // 0xE000E400-0xE000E4EF
    uint32_t RESERVED5[644];
    // Software Trigger Interrupt Register
    volatile uint32_t STIR; // 0xE000EF00
} NVIC_RegDef_t;

typedef struct
{
    volatile uint32_t CSR;   // 0x00
    volatile uint32_t RVR;   // 0x04
    volatile uint32_t CVR;   // 0x08
    volatile uint32_t CALIB; // 0x0C
} SYST_RegDef_t;

#define NVIC_BASE (0xE000E100UL)
#define SYST_BASE (0xE000E010UL)

#define NVIC ((NVIC_RegDef_t *)NVIC_BASE)
#define SYST ((SYST_RegDef_t *)SYST_BASE)

__attribute__((always_inline)) static inline void __DMB(void)
{
    __asm volatile("dmb" ::: "memory");
}

#endif