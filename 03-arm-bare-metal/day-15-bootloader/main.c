#include "stm32f411.h"
#include "flash_config.h"
#include "uart/uart.h"
#include "systick/systick.h"

void flash_bsy_checking(void)
{
    // check if the flash memory operation is in progress
    // bit 16 BSY of SR is 1 while the operation is in progress, 0 - when the operation finishes or an error occurs
    while (FLASH->SR & (1UL << 16U)) // exits when the BSY is 0
        ;
}

void uart_ack_nack_host(UART_HostConfirmation_t value)
{
    while (!(USART2->SR & (1UL << 7U)))
        ;

    USART2->DR = value;
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

FLASH_ReturnTypes_t flash_program(uint32_t address_base, uint32_t *data, uint32_t len)
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
        *(volatile uint32_t *)(address_base + (i * 4)) = data[i];
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

UART_ChunkReceive_ReturnTypes_t uart_chunk_receive_protocol()
{
    // erasing the sector 2 before the first chunk
    flash_erase(FLASH_SNB2);

    SysTick_Init(SYSTICK_FREQUENCY_16MHZ);
    usart2_init();

    uint8_t is_last = 0;      // flag to track the reception of the sentinel packet
    uint8_t is_overflow = 0;  // flag to signal the 16 KB ceiling is hit, the incoming image as well as the next ones are going to be rejected
    uint8_t is_retries = 0;   // flag to signal the byte has exhausted 3 retries
    uint8_t is_corrupted = 0; // flag to signal the chunk got corrupted

    uint8_t did_retry_hit = 0;

    UART_Reception_States_t reception_state = UART_RECEPTION;
    UART_Chunks_States_t chunk_state = UART_START_BYTE;
    UART_ChunkReceive_Layout_t packet;

    uint8_t payload_index = 0;
    uint8_t retries_counter = 0;
    uint32_t all_payload_bytes = 0;
    uint8_t checksum_received = 0;

    uint32_t chunks_received = 0;

    while (!is_last)
    {

        if (reception_state == UART_RECEPTION)
        {

            // waiting for the read data register to receive a byte
            uint32_t start = SysTick_GetTick();

            while (!(USART2->SR & (1UL << 5U)))
            {
                if ((SysTick_GetTick() - start) >= 2)
                {
                    retries_counter++;

                    if (retries_counter >= 3U)
                    {
                        is_retries = 1;
                        break;
                    }

                    did_retry_hit = 1;
                    break;
                }
            }

            if (is_retries)
            {
                chunk_state = UART_START_BYTE;
                reception_state = UART_CHECKING; // in the Checking NACK the bit
                continue;
            }

            if (did_retry_hit)
            {
                chunk_state = UART_START_BYTE;
                reception_state = UART_CHECKING;
                payload_index = 0;
                continue;
            }

            retries_counter = 0; // retries reload

            switch (chunk_state)
            {
            case UART_START_BYTE:
                packet.start_byte = USART2->DR;
                chunk_state = UART_PAYLOAD_LEN;
                break;

            case UART_PAYLOAD_LEN:
                uint8_t payload_len = USART2->DR;

                // payload_len = 0 -> we received the end-of-transfer packet
                if (payload_len == 0)
                {
                    is_last = 1;
                }
                else
                {
                    packet.payload_len = payload_len;
                    payload_index = 0;
                    chunk_state = UART_PAYLOAD;
                }

                break;

            case UART_PAYLOAD:
                all_payload_bytes++;
                packet.payload[payload_index++] = USART2->DR;

                // 128 because payload_index has already incremented from 127 (the last element since we began from 0) to 128
                if (payload_index == 128)
                {
                    checksum_received = 0;
                    chunk_state = UART_CHECKSUM;
                }

                break;

            case UART_CHECKSUM:
                checksum_received++;

                uint8_t byte = USART2->DR;

                // reconstruction of 16 bit length checksum
                if (checksum_received == 1)
                {
                    packet.checksum = ((uint16_t)byte << 8U);
                }
                else if (checksum_received == 2)
                {
                    packet.checksum |= ((uint16_t)byte << 0U);
                    chunk_state = UART_END_BYTE;
                }

                break;

            case UART_END_BYTE:
                chunks_received++;
                packet.end_byte = USART2->DR;

                reception_state = UART_CHECKING;
                chunk_state = UART_START_BYTE;
                break;
            }
        }

        else if (reception_state == UART_CHECKING)
        {
            uint16_t payload_sum = 0;

            if (is_retries)
            {
                // send NACK to the UART and go to the main()
                uart_ack_nack_host(UART_NACK_ABORT);
                break;
            }

            if (did_retry_hit)
            {
                // send NACK to the UART for the chunk and expect it to retransmit the entire 132-byte packet from scratch
                uart_ack_nack_host(UART_NACK_RETRY);

                did_retry_hit = 0;
                reception_state = UART_RECEPTION;
                continue;
            }

            if (all_payload_bytes > 16384)
            {
                // 16384 = 128 payloads with 32-word long payloads
                // sector 2 is full
                // reject the image

                is_overflow = 1;
            }

            if (!is_overflow)
            {
                for (uint8_t i = 0; i < 128; i++)
                {
                    payload_sum += packet.payload[i];
                }

                uint16_t covered_sum = packet.start_byte + packet.payload_len + payload_sum;

                if (covered_sum == packet.checksum)
                {

                    uint32_t flash_payload[32];

                    for (uint8_t i = 0, byte_count = 0; byte_count < 128 && i < 32; byte_count += 4, i++)
                    {
                        // explicit cast from uint8_t to uint32_t to prevent implicit case to int
                        flash_payload[i] = ((uint32_t)packet.payload[byte_count]) | (((uint32_t)packet.payload[byte_count + 1]) << 8U) | (((uint32_t)packet.payload[byte_count + 2]) << 16U) | (((uint32_t)packet.payload[byte_count + 3]) << 24U);
                    }

                    FLASH_ReturnTypes_t flash_result = flash_program((FLASH_SECTOR2 + 128 * (chunks_received - 1)), flash_payload, 32);

                    if (flash_result == FLASH_OK)
                    {
                        // acknowledge the packet
                        uart_ack_nack_host(UART_ACK_OK);
                    }
                    else
                    {
                        // FLASH_ERROR
                        uart_ack_nack_host(UART_NACK_FLASH);
                    }
                }
                else
                {
                    // the calculated checksum does not agree with the received checksum
                    // the packet got corrupted
                    is_corrupted = 1;
                    uart_ack_nack_host(UART_NACK_CORRUPTED);
                    // the UART_NACK_CORRUPTED should be treated as UART_NACK_RETRY - the host should send the chunk from scratch
                    continue;
                }
            }
            else
            {
                // NACK the chunk since it is overflown
                uart_ack_nack_host(UART_NACK_OVERFLOW);
                break; // exit the outer loop
            }
        }
    }

    // overflow happened before the byte has exhausted 3 retries
    if (is_corrupted)
    {
        return UART_CORRUPTED;
    }
    else if (is_overflow)
    {
        return UART_OVERFLOW_ABORT;
    }
    else if (is_retries)
    {
        return UART_RETRIES_ABORT;
    }
    else
    {
        return UART_OK;
    }
}

void main(void)
{
    // GPIO-check on a boot
    // pin PB13 with 5 kOhms external resistor

    // enable the RCC clock for the port B
    RCC->AHB1ENR |= (1UL << 1U);

    // set MODER for PB13 to 00 (input)
    // pin 13 has bits 27:26 in the MODER register layout
    // for port B the reset state: 0x00000280 -bits 7 and 9 are 1
    // so there is no explicit need to clear the bits 27:26 of MODER

    // but still, to be 100% sure, let's clear the bits 27:26 of the GPIOB_MODER
    // 11 = 0x3
    GPIOB->MODER &= ~(0x3UL << 26U);

    while (1)
    {
        // there is a tactile push-button connected to GND - a pull up - so the pin reads 1 when open and reads 0 when the circuit is closed
        // to read the logic value of the PB13, we are using GPIO_IDR

        // if the logic value is 1, then the button is not pressed -> jump to application
        if (GPIOB->IDR & (1UL << 13U))
        {
        }
        // if the value is 0 -> stay in bootloader
        else
        {
        }
    }

    return;
}