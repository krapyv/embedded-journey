#include "osal_time.h"
#include "systick.h"

uint32_t osal_get_ms(void)
{
    return SysTick_GetTick();
}

void osal_delay_ms(uint32_t ms)
{
    SysTick_Delay_ms(ms);
}