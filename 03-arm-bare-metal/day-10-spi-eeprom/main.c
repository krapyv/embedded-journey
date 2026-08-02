#include <stdio.h>
#include "spi.h"
#include "led.h"
#include "m9520.h"
#include "systick.h"

int main()
{
    spi_init(SPI_BR_2);

    // PA0
    LED_HandleTypeDef correct_led = {
        .pin = 0,
        .rcc_bit = 0,
        .odr_reg = &(GPIOA->ODR),
        .moder_reg = &(GPIOA->MODER),
        .rcc_clk_reg = &(RCC->AHB1ENR)};

    // PB1
    LED_HandleTypeDef fault_led = {
        .pin = 1,
        .rcc_bit = 1,
        .odr_reg = &(GPIOB->ODR),
        .moder_reg = &(GPIOB->MODER),
        .rcc_clk_reg = &(RCC->AHB1ENR)};

    led_init(&correct_led);
    led_init(&fault_led);

    // MSB: 0000 => 0x0
    // LSB: 01010000 => 0x50
    uint8_t address_arr[2] = {0x00, 0x50};

    uint8_t data_arr[1] = {0x6F};
    uint8_t read_arr[1] = {0};

    SysTick_Init(SYSTICK_FREQUENCY_16MHZ);

    // the datasheet gives endurance in write cycles per group of 4 bytes at 4 000 000 cycles.
    // an infinite loop with 5 ms cycle takes 5.56 hours approximately to exhaust this value
    // 200 cycles per second

    // so we are changing while (1) to for ()
    for (uint8_t i = 0; i < 30; i++)
    {
        eeprom_write_byte(address_arr, data_arr, 1U);

        eeprom_read(address_arr, read_arr, 1U);

        // tested the fault branch
        // data_arr[0] = 0xF1;

        if (data_arr[0] == read_arr[0])
        {
            led_on(&correct_led);

            SysTick_Delay_ms(300);

            led_off(&correct_led);

            SysTick_Delay_ms(300);
        }
        else
        {
            // if there is a fault during test,  halt the execution, signal the fault
            while (1)
            {
                led_on(&fault_led);

                SysTick_Delay_ms(300);

                led_off(&fault_led);

                SysTick_Delay_ms(300);
            }
        }

        read_arr[0] = 0;

        if (data_arr[0] == 0x6F)
        {
            data_arr[0] = 0x30;
        }
        else if (data_arr[0] == 0x30)
        {
            data_arr[0] = 0xF1;
        }
        else
        {
            data_arr[0] = 0x6F;
        }
    }

    // after the loop finishes
    // simultaneous blinking to signal that now the communication with eeprom is done
    while (1)
    {
        led_on(&correct_led);
        led_on(&fault_led);

        SysTick_Delay_ms(300);

        led_off(&correct_led);
        led_off(&fault_led);

        SysTick_Delay_ms(300);
    }

    return 0;
}