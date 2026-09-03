#include "stm32f411.h"

void flash_bsy_checking(void)
{
    // check if the flash memory operation is in progress
    // bit 16 BSY of SR is 1 while the operation is in progress, 0 - when the operation finishes or an error occurs
    while (FLASH->SR & (1UL << 16U)) // exits when the BSY is 0
        ;
}

void flash_program(uint32_t *addresses, uint32_t *data, uint32_t len)
{
    flash_bsy_checking();

    // check whether the FLASH CR is locked (bit 31 is set to 1)
    if (FLASH->CR & (1 << 31))
    {
        // FLASH unlock
        // The keys 1 and 2 must be programmed consecutively to unlock the FLASH_CR register and allow programming/erasing it
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }

    // activate Flash programming, bit 0 PG
    FLASH->CR |= (1UL << 0U);

    // set the program size

    // since the PSIZE value takes 2 bits, clear the range
    // 11 = 0x3
    FLASH->CR &= ~(0x3 << 8);

    // set the value of x32 = 10
    // 10 = 0x2
    FLASH->CR |= (0x2 << 8);

    // word-write loop
    for (uint32_t i = 0; i < len; i++)
    {
        // write a word to the FLASH memory
        *(uint32_t *)(addresses[i]) = data[i];

        flash_bsy_checking();
    }

    // after all words have been written to FLASH, disable the PG
    FLASH->CR &= ~(1UL << 0U);
}

void flash_erase(FLASH_SNB_t sector_num)
{
    flash_bsy_checking();

    // check whether the FLASH CR is locked (bit 31 is set to 1)
    if (FLASH->CR & (1 << 31))
    {
        // FLASH unlock
        // The keys 1 and 2 must be programmed consecutively to unlock the FLASH_CR register and allow programming/erasing it
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }

    // activate Sector Erase
    FLASH->CR |= (1UL << 1U);

    // select the sector number to erase

    // the SNB takes 4 bits [6:3]
    // firstly clear the range
    // 1111 = 2^3 + 2^2 + 2^1 + 2^0 = 8 + 4 + 2 + 1 = 15 = 0xF
    FLASH->CR &= ~(0xFUL << 3U);

    // set the value
    FLASH->CR |= sector_num;

    // set the STRT bit 16 to 1 to trigger an erase operation
    FLASH->CR |= (1UL << 16U);

    flash_bsy_checking();

    // once the BSY bit is cleared (the erase operation has ended), clear the STRT bit in CR
    FLASH->CR &= ~(1UL << 16U);
}

void main(void)
{
    return;
}