#include "iwdg.h"

void IWDG_kick(IWDG_HandleTypeDef *wdg)
{
    // kick the watchdog
    *(wdg->IWDG_BASE_REG + 0) = 0xAAAA;
}

void IWDG_init(IWDG_HandleTypeDef *wdg)
{
    // start the watchdog
    // offset: 0x00
    *(wdg->IWDG_BASE_REG + 0) = 0xCCCC;

    // enable access to the IWDG_PR and IWDG_RLR
    // offset: 0x00
    *(wdg->IWDG_BASE_REG + 0) = 0x5555;

    // set the prescaler register
    // offset: 0x04
    *(wdg->IWDG_BASE_REG + 1) = wdg->prescaler_val;

    // set the reload register
    // offset: 0x08
    *(wdg->IWDG_BASE_REG + 2) = wdg->reload_val;

    // introduce defensive synchronization loop with timeout
    // this ensures that even if the peripheral (e.g. LSI oscillator) is completely dead, the CPU could escape the loop
    volatile uint32_t timeout_counter = 100000;

    // checking if Status register is 0, so both bits (PVU and RVU) are clear and the peripheral is completely idle
    // we are masking out the status register with 0x03 to prevent any possible future changes in the architecture of the SR
    // we do care only about bits 0 and 1 (PVU and RVU), so we can clear all other bits 2-31 (by setting them to 0)
    while (((*(wdg->IWDG_BASE_REG + 3) & 0x03) != 0x00) && (timeout_counter > 0))
    {
        timeout_counter--;
    }

    // if the counter drops to 0, it means the peripheral did not respond.
    if (timeout_counter == 0)
    {
        return;
    }

    // refresh the counter and safely lock the configuration window
    IWDG_kick(wdg);
}
