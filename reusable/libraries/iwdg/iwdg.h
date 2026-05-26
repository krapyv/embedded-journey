#ifndef IWDG_H
#define IWDG_H

#include <stdint.h>

typedef enum
{
    IWDG_PRESCALER_4 = 0x00,       // 0b000
    IWDG_PRESCALER_8 = 0x01,       // 0b001
    IWDG_PRESCALER_16 = 0x02,      // 0b010
    IWDG_PRESCALER_32 = 0x03,      // 0b011
    IWDG_PRESCALER_64 = 0x04,      // 0b100
    IWDG_PRESCALER_128 = 0x05,     // 0b101
    IWDG_PRESCALER_256 = 0x06,     // 0b110
    IWDG_PRESCALER_256_ALT = 0x07, // 0b111
} IWDG_Prescaler_t;

typedef struct
{
    volatile uint32_t *IWDG_BASE_REG;
    IWDG_Prescaler_t prescaler_val;
    uint32_t reload_val;
} IWDG_HandleTypeDef;

void IWDG_init(IWDG_HandleTypeDef *wdg);
void IWDG_kick(IWDG_HandleTypeDef *wdg);

#endif