#include "spi.h"
#include <stdio.h>

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

    /* --- ODR --- */
    GPIOA->ODR |= (1 << 4);

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

    // since PA7, PA6, PA5 are in the range 0-7, we use GPIO_AFRL

    /* --- AFRL --- */
    // AFRL/AFRH gives 4 bits per pin: PA7 (31:28), PA6 (27:24), PA5 (23:20)

    // clear the bits
    // 1111 = 2^3 + 2^2 + 2^1 + 2^0 = 8 + 4 + 2 + 1 = 15 = 0xF
    GPIOA->AFRL &= ~((0xF << 28) | (0xF << 24) | (0xF << 20));

    // set the bits
    // 0101 = 2^2 + 2^0 = 4 + 1 = 5 = 0x5
    GPIOA->AFRL |= ((0x5 << 28) | (0x5 << 24) | (0x5 << 20));

    /* --- OSPEEDR --- */
    // since only SCK (PA5) and MOSI (PA7) are driven by the STM32F411 board
    // they should keep up with the SPI frequency of 8 MHz

    // by default OSPEEDR has the value of 00, with the max frequency (CL = 50 pF, VDD >= 2.70 V) of 4 MHz

    // the parasitic capacitance of the breadboard circuit:
    // SCK PA5: this row sits next to another active line, it is approximately 3 pF
    // Jumper wire i am using is 20 cm. 2.5 cm is approximately 1 pF, so it is 8 pF
    // MCU pin (STM32 GPIO Input/output capacitance = approximately 5 pF (according to the datasheet)
    // EEPROM Pin: accordin to the datasheet it is 6 pF

    // so we can sum them up: 3 + 8 + 5 + 6 = 11 + 11 = 22 pF

    // so we choose the 01 option that gives under CL = 50 pF, VDD >= 2.70 V, the frequency of 25 MHz >> 8 MHz

    // OSPEEDR gives 2 bits per pin: pin PA5 takes bits 11:10, PA7 takes bits 15:14
    // clear the bits
    // 11 = 0x3

    GPIOA->OSPEEDR &= ~((0x3 << 14) | (0x3 << 10));

    // set the bits
    // 01 = 0x1
    GPIOA->OSPEEDR |= ((0x1 << 14) | (0x1 << 10));
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

void spi_transfer(uint8_t *tx, uint8_t *rx, uint8_t len)
{
    // first of all, CS assert (ODR low for PA4)
    GPIOA->ODR &= ~(1 << 4);

    uint8_t dummy_rx;
    (void)dummy_rx;

    uint8_t dummy_tx = 0xFF;
    (void)dummy_tx;

    // check if the TX DR is empty
    while (!(SPI1->SR & (1 << 1)))
        ;

    for (uint8_t i = 0; i < len; i++)
    {
        // write the byte in DR
        if (tx == NULL)
        {
            SPI1->DR = dummy_tx;
        }
        else
        {
            SPI1->DR = tx[i];
        }

        // wait until the incoming byte lands in the RX DR
        while (!(SPI1->SR & (1 << 0)))
            ;

        if (rx == NULL)
        {
            dummy_rx = SPI1->DR;
        }
        else
        {
            rx[i] = SPI1->DR;
        }
    }

    // after the last incoming byte was read
    // check BSY bit in SR for 0
    while (SPI1->SR & (1 << 7))
        ;

    // deselect the EEPROM
    // ODR high for PA4
    GPIOA->ODR |= (1 << 4);

    return;
}