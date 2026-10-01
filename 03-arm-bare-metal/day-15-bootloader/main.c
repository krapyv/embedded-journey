#include "stm32f411.h"
#include "core_cm4.h"
#include "flash_config.h"
#include "uart/uart.h"
#include "systick/systick.h"

#define SRAM_START 0x20000000U
#define SRAM_SIZE (128U * 1024U) // 128 KB
#define SRAM_END (SRAM_START + SRAM_SIZE)

#define APP_FLASH_START 0x08008000U // start of the sector 2
#define APP_FLASH_END 0x0800BFFF    // end of the sector 2

#define CRC_INIT 0xFFFF
#define CRC_XOROUT 0xFFFF
#define CRC_POLY 0x1021
#define CRC_REFLECTED_POLY 0x8408

// global instance of HardFault_Struct_t
static volatile HardFault_Struct_t hardfault_dump;

const uint16_t table[256] = {0x0, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf, 0x8c48,
                             0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7, 0x1081, 0x108,
                             0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e, 0x9cc9, 0x8d40, 0xbfdb,
                             0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876, 0x2102, 0x308b, 0x210, 0x1399,
                             0x6726, 0x76af, 0x4434, 0x55bd, 0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e,
                             0xfae7, 0xc87c, 0xd9f5, 0x3183, 0x200a, 0x1291, 0x318, 0x77a7, 0x662e,
                             0x54b5, 0x453c, 0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd,
                             0xc974, 0x4204, 0x538d, 0x6116, 0x709f, 0x420, 0x15a9, 0x2732, 0x36bb,
                             0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3, 0x5285,
                             0x430c, 0x7197, 0x601e, 0x14a1, 0x528, 0x37b3, 0x263a, 0xdecd, 0xcf44,
                             0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72, 0x6306, 0x728f, 0x4014,
                             0x519d, 0x2522, 0x34ab, 0x630, 0x17b9, 0xef4e, 0xfec7, 0xcc5c, 0xddd5,
                             0xa96a, 0xb8e3, 0x8a78, 0x9bf1, 0x7387, 0x620e, 0x5095, 0x411c, 0x35a3,
                             0x242a, 0x16b1, 0x738, 0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862,
                             0x9af9, 0x8b70, 0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e,
                             0xf0b7, 0x840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
                             0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036, 0x18c1,
                             0x948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e, 0xa50a, 0xb483,
                             0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5, 0x2942, 0x38cb, 0xa50,
                             0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd, 0xb58b, 0xa402, 0x9699, 0x8710,
                             0xf3af, 0xe226, 0xd0bd, 0xc134, 0x39c3, 0x284a, 0x1ad1, 0xb58, 0x7fe7,
                             0x6e6e, 0x5cf5, 0x4d7c, 0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1,
                             0xa33a, 0xb2b3, 0x4a44, 0x5bcd, 0x6956, 0x78df, 0xc60, 0x1de9, 0x2f72,
                             0x3efb, 0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
                             0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0xd68, 0x3ff3, 0x2e7a, 0xe70e,
                             0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1, 0x6b46, 0x7acf,
                             0x4854, 0x59dd, 0x2d62, 0x3ceb, 0xe70, 0x1ff9, 0xf78f, 0xe606, 0xd49d,
                             0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330, 0x7bc7, 0x6a4e, 0x58d5, 0x495c,
                             0x3de3, 0x2c6a, 0x1ef1, 0xf78};

uint16_t reflect(uint16_t byte, uint8_t size)
{
    uint16_t reversed_byte = 0;

    for (uint8_t i = 0; i < size; i++)
    {
        uint16_t bit = byte & 1;

        reversed_byte = (reversed_byte << 1) | bit;

        byte = (byte >> 1);
    }

    return reversed_byte;
}

// NOTE: the function used to get the lookup table of 256 elements
// void populate_lookup_table()
// {
//     uint16_t reg = 0;

//     for (uint16_t i = 0; i < 256; i++)
//     {
//         reg = i;

//         for (uint8_t j = 0; j < 8; j++)
//         {
//             uint16_t bit = reg & 1;

//             if (bit == 1)
//             {
//                 reg = (reg >> 1) ^ CRC_REFLECTED_POLY;
//             }
//             else
//             {
//                 reg = (reg >> 1);
//             }
//         }

//         // store the value in the table
//         table[i] = reg;
//     }
// }

uint16_t crc16_bit(const uint8_t *data, uint16_t len)
{
    uint16_t reg = CRC_INIT;

    for (uint16_t i = 0; i < len; i++)
    {
        reg ^= data[i];

        for (uint8_t j = 0; j < 8; j++)
        {
            uint16_t bit = reg & 1;

            if (bit == 1)
            {
                reg = (reg >> 1) ^ CRC_REFLECTED_POLY;
            }
            else
            {
                reg = (reg >> 1);
            }
        }
    }

    reg ^= CRC_XOROUT;

    return reg;
}

uint16_t crc16_table(const uint8_t *data, uint16_t len)
{
    uint16_t reg = CRC_INIT;
    uint8_t index = 0;

    for (uint16_t i = 0; i < len; i++)
    {
        index = reg ^ data[i];

        reg = (reg >> 8) ^ table[index];
    }

    reg ^= CRC_XOROUT;

    return reg;
}

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

__attribute__((naked)) void HardFault_Handler(void)
{
    __asm volatile(
        "tst lr, #4     \n"                 // bit 2 of EXC_RETURN tells which SP was used; #4 is 0b100, i.e. bit 2 set
        "ite eq         \n"                 // sets up an if/else over the next two instructions
        "mrseq r0, msp  \n"                 // if lr & 0x4 == 0 -> EQ is true -> MSP was used
        "mrsne r0, psp  \n"                 // if lr & 0x4 != 0 -> NE is true -> PSP was used
        "ldr r1, =HardFault_Handler_C   \n" // ldr loads a register with a value from a PC-relative memory address
        "bx r1  \n"                         // tail-jump into a normal C function, passing the stack pointer as the argument
    );

    // b HardFault_Handler_C would do the same job as
    // ldr r1, =HardFault_Handler_C
    // bx r1
    // since HardFault_Handler_C is a fixed, statically-known symbol (not a runtime-computed address)
}

void HardFault_Handler_C(uint32_t *faultStackedRegs)
{
    // faultStackedRegs[0..7] = R0, R1, R2, R3, R12, LR, PC, xPSR
    volatile uint32_t r0 = faultStackedRegs[0];
    volatile uint32_t r1 = faultStackedRegs[1];
    volatile uint32_t r2 = faultStackedRegs[2];
    volatile uint32_t r3 = faultStackedRegs[3];
    volatile uint32_t r12 = faultStackedRegs[4];
    volatile uint32_t lr = faultStackedRegs[5];
    volatile uint32_t pc = faultStackedRegs[6];
    volatile uint32_t xpsr = faultStackedRegs[7];

    // SCB->CFSR
    volatile uint32_t cfsr = SCB->CFSR;

    // SCB->HFSR
    volatile uint32_t hfsr = SCB->HFSR;

    // MMARVALID
    bool mmfar_valid = (SCB->CFSR & (1UL << 7U)); // 1 - true, 0 - false
    // MMARVALID
    bool bfar_valid = (SCB->CFSR & (1UL << 15U)); // 1 - true, 0 - false

    // SCB->MMAR
    volatile uint32_t mmfar = SCB->MMFAR;

    // SCB->BFAR
    volatile uint32_t bfar = SCB->BFAR;

    // HFSR
    bool hfsr_forced = (hfsr & (1UL << 30U));

    hardfault_dump.PC = pc;
    hardfault_dump.R0 = r0;
    hardfault_dump.R1 = r1;
    hardfault_dump.R2 = r2;
    hardfault_dump.R3 = r3;
    hardfault_dump.R12 = r12;
    hardfault_dump.LR = lr;
    hardfault_dump.xPSR = xpsr;
    hardfault_dump.CFSR = cfsr;
    hardfault_dump.HFSR = hfsr;
    hardfault_dump.hfsr_forced = hfsr_forced;
    hardfault_dump.mmfar_valid = mmfar_valid;
    hardfault_dump.bfar_valid = bfar_valid;

    if (mmfar_valid)
    {
        hardfault_dump.MMFAR = mmfar;
    }
    if (bfar_valid)
    {
        hardfault_dump.BFAR = bfar;
    }

    while (1)
    {
    }
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
    usart2_init();

    flash_erase(FLASH_SNB2);
    SysTick_Init(SYSTICK_FREQUENCY_16MHZ);

    uart_ack_nack_host(UART_ACK_START);

    uint8_t is_last = 0;      // flag to track the reception of the sentinel packet
    uint8_t is_overflow = 0;  // flag to signal the 16 KB ceiling is hit, the incoming image as well as the next ones are going to be rejected
    uint8_t is_retries = 0;   // flag to signal the byte has exhausted 3 retries
    uint8_t is_corrupted = 0; // flag to signal the chunk got corrupted
    uint8_t is_flash_error = 0;

    uint8_t did_retry_hit = 0;

    UART_Reception_States_t reception_state = UART_RECEPTION;
    UART_Chunks_States_t chunk_state = UART_START_BYTE;
    UART_ChunkReceive_Layout_t packet;

    uint8_t payload_index = 0;

    uint32_t all_payload_bytes = 0;
    uint8_t checksum_received = 0;
    uint32_t chunks_received = 0;

    uint8_t retries_counter = 0;
    uint8_t corrupted_counter = 0;
    uint8_t error_counter = 0;

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
                    uart_ack_nack_host(UART_ACK_OK);
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
                chunks_received--;
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

                    corrupted_counter = 0;

                    for (uint8_t i = 0, byte_count = 0; byte_count < 128 && i < 32; byte_count += 4, i++)
                    {
                        // explicit cast from uint8_t to uint32_t to prevent implicit case to int
                        flash_payload[i] = ((uint32_t)packet.payload[byte_count]) | (((uint32_t)packet.payload[byte_count + 1]) << 8U) | (((uint32_t)packet.payload[byte_count + 2]) << 16U) | (((uint32_t)packet.payload[byte_count + 3]) << 24U);
                    }

                    FLASH_ReturnTypes_t flash_result = flash_program((FLASH_SECTOR2 + 128 * (chunks_received - 1)), flash_payload, 32);

                    if (flash_result == FLASH_OK)
                    {
                        // acknowledge the packet
                        error_counter = 0;
                        uart_ack_nack_host(UART_ACK_OK);

                        reception_state = UART_RECEPTION;
                        payload_index = 0;
                        continue;
                    }
                    else
                    {
                        // FLASH_ERROR
                        error_counter++;
                        chunks_received--;
                        uart_ack_nack_host(UART_NACK_FLASH);

                        if (error_counter >= 3U)
                        {
                            is_flash_error = 1;
                            break;
                        }

                        reception_state = UART_RECEPTION;
                        payload_index = 0;
                        continue;
                    }
                }
                else
                {
                    // the calculated checksum does not agree with the received checksum
                    // the packet got corrupted
                    corrupted_counter++;
                    chunks_received--;
                    uart_ack_nack_host(UART_NACK_CORRUPTED);
                    // the UART_NACK_CORRUPTED should be treated as UART_NACK_RETRY - the host should send the chunk from scratch

                    if (corrupted_counter >= 3U)
                    {
                        is_corrupted = 1;
                        break;
                    }

                    reception_state = UART_RECEPTION;
                    payload_index = 0;
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
    else if (is_flash_error)
    {
        return UART_FLASH_ERROR;
    }
    else
    {
        return UART_OK;
    }
}

bool is_valid_application(uint32_t app_sp, uint32_t app_reset_handler)
{
    // verify Stack Pointer points inside SRAM1 (0x2000 0000 - 0x2002 0000)

    if ((app_sp < SRAM_START) || (app_sp > SRAM_END))
    {
        return false;
    }

    // 0x7 = 0111
    if ((app_sp & 0x7U) != 0U)
    {
        return false;
    }

    // verify Reset Handler has Thumb bit set (Bit 0 must be 1, not 0 (32-bit ARM mode))
    if ((app_reset_handler & 0x01U) == 0U)
    {
        return false;
    }

    if ((app_reset_handler < APP_FLASH_START) || (app_reset_handler > APP_FLASH_END))
    {
        return false;
    }

    return true;
}

__attribute__((naked, noreturn)) void jump_to_application(uint32_t app_sp, uint32_t app_reset_handler)
{
    // r0 contains app_sp, r1 contains app_reset_handler

    // 1. Set the Main Stack Pointer to Application's Stack Pointer

    // 2. Jump to Application's Reset_Handler

    __asm__ volatile(
        "MSR msp, r0 \n"
        "BX r1 \n");
}

SP_Validation_t execute_user_application()
{
    // Flash sector 0 and 1 belong to the bootloader
    // Flash sector 2 belongs to the application

    // Application Vector Table sits at sector 2 Start Address
    uint32_t app_vector_table = 0x08008000;

    // extract Main Stack Pointer (1st entry in Vector Table)
    uint32_t app_msp = *(volatile uint32_t *)(app_vector_table); // turns a raw hexadecimal memory address into a directly writable/readable 32-bit hardware register

    // extract Reset Handler address (2nd entry in Vector Table)
    uint32_t app_reset_handler = *(volatile uint32_t *)(app_vector_table + 4);

    // SP and Reset Handler validation
    if (!is_valid_application(app_msp, app_reset_handler))
    {
        return SP_Validation_ERROR;
    }

    // disable SysTick peripheral and interrupts

    // bit 16 COUNTFLAG is read only, bit 2 CLKSOURCE is irrelevant when the SysTick is disabled, bit 1 TICKINT is 0, so no exceptio requests are possible, bit 0 ENABLE is 0, so the counter is disabled
    SYST->CSR = 0;
    SYST->RVR = 0;
    SYST->CVR = 0;

    // relocate Vector Offset Register (VTOR) to App Start
    SCB->VTOR = app_vector_table;

    // execute Naked Jump that never returns
    jump_to_application(app_msp, app_reset_handler);
}

// void main(void)
// {
//     // GPIO-check on a boot
//     // pin PB13 with 5 kOhms external resistor

//     // enable the RCC clock for the port B
//     RCC->AHB1ENR |= (1UL << 1U);

//     // set MODER for PB13 to 00 (input)
//     // pin 13 has bits 27:26 in the MODER register layout
//     // for port B the reset state: 0x00000280 -bits 7 and 9 are 1
//     // so there is no explicit need to clear the bits 27:26 of MODER

//     // but still, to be 100% sure, let's clear the bits 27:26 of the GPIOB_MODER
//     // 11 = 0x3
//     GPIOB->MODER &= ~(0x3UL << 26U);

//     bool logic_state = (GPIOB->IDR & (1UL << 13U));

//     while (1)
//     {
//         // there is a tactile push-button connected to GND - a pull up - so the pin reads 1 when open and reads 0 when the circuit is closed
//         // to read the logic value of the PB13, we are using GPIO_IDR

//         // if the logic value is 1, then the button is not pressed -> jump to application
//         if (logic_state)
//         {
//             if (execute_user_application() == SP_Validation_ERROR)
//             {
//                 // SP validation failed
//                 uart_chunk_receive_protocol();
//             }
//         }
//         // if the value is 0 -> stay in bootloader
//         else
//         {
//             if (uart_chunk_receive_protocol() == UART_OK)
//             {
//                 execute_user_application();
//             }
//         }
//     }

//     return;
// }

void main()
{
    const uint8_t test_packet[128] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
        0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F};

    uint16_t crc_res_table = crc16_table(test_packet, 128U);

    uint16_t crc_res_bit = crc16_bit(test_packet, 128U);

    (void)crc_res_table;
    (void)crc_res_bit;
    while (1)
    {
    }
}