#include "spi.h"

void spi_init()
{
    // NSS1 - PA4, SCK1 - PA5, MISO1 - PA6, MOSI1 - PA7

    // make sure the SPE bit (CR1, Bit 6) is disabled
    SPI1->CR1 &= ~(1 << 6);

    /* ----- RCC Configuration ----- */

    // enable GPIOA RCC clock
    RCC->AHB1ENR |= (1 << 0);

    // enable SPI1 RCC clock
    RCC->APB2ENR |= (1 << 12);

    /* ----- RCC Configuration ----- */

    /* ----- GPIO Configuration ----- */
    // PA5, PA6, PA7 - MODER: Alternate function mode (10) - AF5 (0101)
    // PA4 - MODER: General purpose output mode (01) - OTYPER: 0 - output push-pull

    /* --- MODER --- */
    // clear the bits
    // MODER gives 2 bits per pin
    // PA4 - bits 9:8, PA5 - bits 11:10, PA6 - bits 13:12, PA7 - bits 15:14
    // 11 = 0x3
    GPIOA->MODER &= ~((0x3 << 14) | (0x3 << 12) | (0x3 << 10) | (0x3 << 8));

    // set the bits
    // 10 = 0x2, 01 = 0x1
    GPIOA->MODER |= ((0x2 << 14) | (0x2 << 12) | (0x2 << 10) | (0x1 << 8));

    /* --- OTYPER --- */
    // clear the bit 4
    GPIOA->OTYPER &= ~(1 << 4);

    /* --- ODR --- */
    GPIOA->ODR |= (1 << 4);
    // since PA7, PA6, PA5 are in the range 0-7, we use GPIO_AFRL

    /* --- AFRL --- */
    // AFRL/AFRH gives 4 bits per pin: PA7 (31:28), PA6 (27:24), PA5 (23:20)

    // clear the bits
    // 1111 = 2^3 + 2^2 + 2^1 + 2^0 = 8 + 4 + 2 + 1 = 15 = 0xF
    GPIOA->AFRL &= ~((0xF << 28) | (0xF << 24) | (0xF << 20));

    // set the bits
    // 0101 = 2^2 + 2^0 = 4 + 1 = 5 = 0x5
    GPIOA->AFRL |= ((0x5 << 28) | (0x5 << 24) | (0x5 << 20));

    /* ----- GPIO Configuration ----- */

    /* ----- SPI Configuration ----- */
    uint32_t cr1_snap = SPI1->CR1;

    // clear CPHA (Bit 0) and CPOL (Bit 1) for the mode 0
    cr1_snap &= ~((1 << 1) | (1 << 0));

    // set SSM (bit 9), SSI (bit 8) and  MSTR (bit 2) to 1
    cr1_snap |= ((1 << 9) | (1 << 8) | (1 << 2));

    // clear BR (bits 5:3) and DF (bit 11)
    // 111 = 2^2 + 2^1 + 2^0 = 7 = 0x7;
    cr1_snap &= ~((1 << 11) | (0x7 << 3));

    // set SPE (bit 6) to 1
    cr1_snap |= (1 << 6);

    // single write to CR1
    SPI1->CR1 = cr1_snap;
    /* ----- SPI Configuration ----- */

    return;
}