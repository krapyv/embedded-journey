#include "systick.h"
// the NVIC memory map lives here since it is not a STM32 defined peripheral
#include "core_cm4.h"

// the static counter variable to constrain the visiblity scop to this file only
// volatile - to tell the compiller not to optimize away its readings and not assume that the value has not changed if the main thread did not change it
static volatile uint32_t ms_passed = 0;

void SysTick_Init(uint32_t frequency_in_hz)
{
    // first of all, we do not need to set ISER register in NVIC since the SysTick is internal exception to Cortex-M4

    // calculate and set the RELOAD value
    uint32_t reload = (frequency_in_hz / 1000U) - 1U;

    if (reload > SYST_CVR_MAX_RELOAD)
    {
        reload = SYST_CVR_MAX_RELOAD;
    }

    SYST->RVR = reload;

    // clear CVR (Current Value Register) by writing any value that also clears the SYST_CSR COUNTFLAG bit to 0
    SYST->CVR = 0; // CURRENT [23:0], bits [31:24] are reserved, so we can use single-write

    // configure CSR (Control and Status Register) (ENABLE is set at the end of the function, since the counter immediately loads the RELOAD value from the SYST_RVR and starts counting down)
    // set bit 2 CLKSOURSE to 1 (processor clock), bit 1 TICKINT to 1 (counting down to zero assers the SysTick exception request) and bit 0 ENABLE to 1 to enable counter
    SYST->CSR = ((1 << 2) | (1 << 1) | (1 << 0)); // single write since we own all the bits (the COUNTFLAG is cleared by reading)
}

uint32_t SysTick_GetTick(void)
{
    return ms_passed;
}

void SysTick_Delay_ms(uint32_t ms)
{
    // first of all, we need to mark the start timestamp
    uint32_t start = SysTick_GetTick();

    // then we need to wait
    while ((SysTick_GetTick() - start) < ms)
    {
        // do nothing
        // hardware busy-wait loop
    }
}

// the ISR function with the name written in the startup_stm32f411ceux.s
void SysTick_Handler(void)
{
    ms_passed++;
}