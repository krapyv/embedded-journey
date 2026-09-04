#include "stm32f411.h"
#include "flash_config.h"

void flash_bsy_checking(void)
{
    // check if the flash memory operation is in progress
    // bit 16 BSY of SR is 1 while the operation is in progress, 0 - when the operation finishes or an error occurs
    while (FLASH->SR & (1UL << 16U)) // exits when the BSY is 0
        ;
}

FLASH_ReturnTypes_t flash_error_checking()
{
    uint32_t flash_errors = 0;

    // RDERR: Read Protection Error
    // set when an address to be read through the Dbus belongs to a read protected part of the flash
    if (FLASH->SR & FLASH_RDERR)
    {
        flash_errors |= FLASH_RDERR;

        // reset by writing 1
        // single write because bits 8:4 and 1:0 are clear by writing 1, bit 16 BSY is read only and reserved bits 31:17, 15:9, 3:2 have the reset value of 0
        // FLASH_RDERR = (1 << 8), so only the bit 8 is 1, all other bits are 0
        FLASH->SR = FLASH_RDERR;
    }

    // PGSERR: Programming sequence error
    // set when a write access to the flash memory is performed by the code while the control register has not been correctly configured
    if (FLASH->SR & FLASH_PGSERR)
    {
        flash_errors |= FLASH_PGSERR;

        // cleared by writing 1
        // single write because bits 8:4 and 1:0 are clear by writing 1, bit 16 BSY is read only and reserved bits 31:17, 15:9, 3:2 have the reset value of 0
        // FLASH_PGSERR = (1 << 7), so only the bit 7 is 1, all other bits are 0
        FLASH->SR = FLASH_PGSERR;
    }

    // PGPERR: Programming parallelism error
    // set when the size of the access (byte, half-word, word, double word) during the program sequence does not correspond to the parallelism configuration PSIZE (x8, x16, x32, x64)
    if (FLASH->SR & FLASH_PGPERR)
    {
        flash_errors |= FLASH_PGPERR;

        // cleared by writing 1
        // single write because bits 8:4 and 1:0 are clear by writing 1, bit 16 BSY is read only and reserved bits 31:17, 15:9, 3:2 have the reset value of 0
        // FLASH_PGSERR = (1 << 6), so only the bit 6 is 1, all other bits are 0
        FLASH->SR = FLASH_PGPERR;
    }

    // PGAERR: Programming alignment error
    // set when the data to program cannot be contained in the same 128-bit flash memory row
    if (FLASH->SR & FLASH_PGAERR)
    {
        flash_errors |= FLASH_PGAERR;

        // cleared by writing 1
        // single write because bits 8:4 and 1:0 are clear by writing 1, bit 16 BSY is read only and reserved bits 31:17, 15:9, 3:2 have the reset value of 0
        // FLASH_PGAERR = (1 << 5), so only the bit 5 is 1, all other bits are 0
        FLASH->SR = FLASH_PGAERR;
    }

    // WRPERR: Write protection error
    // set when an address to be erased/programmed belongs to a write-protected part of the flash memory
    if (FLASH->SR & FLASH_WRPERR)
    {
        flash_errors |= FLASH_WRPERR;

        // cleared by writing 1
        // single write because bits 8:4 and 1:0 are clear by writing 1, bit 16 BSY is read only and reserved bits 31:17, 15:9, 3:2 have the reset value of 0
        // FLASH_WRPERR = (1 << 4), so only the bit 4 is 1, all other bits are 0
        FLASH->SR = FLASH_WRPERR;
    }

    if (flash_errors != 0)
    {
        // the flash_errors contains all the existing errors, so we can do something with this data in the future (to distinguish what exact errors there are etc)
        return FLASH_ERROR;
    }

    return FLASH_OK;
}

FLASH_ReturnTypes_t flash_program(uint32_t *addresses, uint32_t *data, uint32_t len)
{
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
        *(volatile uint32_t *)(addresses[i]) = data[i];
        // volatile in order to prevent the compiler optimization of writes and its changes of write order

        flash_bsy_checking();
    }

    // after all words have been written to FLASH, disable the PG
    FLASH->CR &= ~(1UL << 0U);

    if (flash_error_checking() != FLASH_OK)
    {
        return FLASH_ERROR;
    }

    return FLASH_OK;
}

FLASH_ReturnTypes_t flash_erase(FLASH_SNB_t sector_num)
{
    // check whether the FLASH CR is locked (bit 31 is set to 1)
    if (FLASH->CR & (1 << 31))
    {
        // FLASH unlock
        // The keys 1 and 2 must be programmed consecutively to unlock the FLASH_CR register and allow programming/erasing it
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }

    // set the program size

    // since the PSIZE value takes 2 bits, clear the range
    // 11 = 0x3
    FLASH->CR &= ~(0x3 << 8);

    // set the value of x32 = 10
    // 10 = 0x2
    FLASH->CR |= (0x2 << 8);

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

    // deactivate Sector Erase
    FLASH->CR &= ~(1UL << 1U);

    if (flash_error_checking() != FLASH_OK)
    {
        return FLASH_ERROR;
    }

    return FLASH_OK;
}

void main(void)
{
    return;
}