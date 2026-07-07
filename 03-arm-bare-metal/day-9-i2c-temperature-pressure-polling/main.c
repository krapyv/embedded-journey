#include <stdio.h>
#include <inttypes.h>
#include "systick.h"
#include "bmp280.h"
#include "i2c.h"
#include "uart.h"

// globally declared vaiable with physically allocated memory in RAM
I2C_HandleTypeDef hi2c;

int main(void)
{
    // define and initialize structs
    I2C_HandleTypeDef hi2c = {
        .channel = I2C_CHANNEL_1,
        .scl_port = GPIOB,
        .scl_pin = 6,
        .sda_port = GPIOB,
        .sda_pin = 7};

    BMP280_HandleTypeDef hbmp = {
        .hi2c = &hi2c,
        .slave_addr = BMP280_I2C_ADDR};

    BMP280_Ctrl_Meas_t meas = {
        .osrs_p = BMP280_OSRS_P_OVRSMP_1,
        .osrs_t = BMP280_OSRS_T_OVRSMP_1,
        .mode = BMP280_FORCED_MODE};

    // peripherals init
    I2C_Init(&hi2c);
    SysTick_Init((uint32_t)SYSTICK_FREQUENCY_16MHZ);
    if (BMP280_Init(&hbmp, meas) != BMP280_OK)
    {
        return 1;
    }
    usart2_init();

    // test loop
    while (1)
    {
        // Firstly read index, then state

        uint8_t current_index = hi2c.index;

        I2C_State_t current_state = hi2c.state;

        if (current_state == I2C_STATE_DONE)
        {
        }

        if (current_state == I2C_STATE_ERROR)
        {
            if (hi2c.error_code == I2C_ERROR_BERR)
            {
                // the SWRST is already 1 and not yet cleared

                // we need to check if the lines are released and the bus is free
                // for that, we need to check GPIO_IDR values for the pins
                uint32_t bus_is_free = 0;

                uint32_t sda_val = 0;
                uint32_t scl_val = 0;

                uint32_t start = SysTick_GetTick();

                while ((SysTick_GetTick() - start) <= 2)
                {
                    sda_val = hi2c.sda_port->IDR & (1 << hi2c.sda_pin);
                    scl_val = hi2c.scl_port->IDR & (1 << hi2c.scl_pin);

                    // if the lines both are 1, then the bus is naturally free
                    if (sda_val && scl_val)
                    {
                        bus_is_free = 1;
                        break;
                    }
                }

                // the waiting IDR loop exits and bus is still not free
                if (!bus_is_free)
                {
                    // gpio clock-banging

                    // first of all, change the MODER for SCL and SDA lines

                    // 1.  SCL to General purpose output mode (01)

                    uint8_t bus_released = 0;

                    // clear the bits (each pin has 2 bits)
                    // 0b0011 = 2^1 + 2^0 = 3 = 0x3
                    hi2c.scl_port->MODER &= ~(0x3 << (hi2c.scl_pin * 2));

                    // set the bits
                    // 01 = 2^0 = 1 = 0x1
                    hi2c.scl_port->MODER |= (0x1 << (hi2c.scl_pin * 2));

                    // make sure the SCL has OTYPER at Open-Drain
                    hi2c.scl_port->OTYPER |= (1 << hi2c.scl_pin);

                    // 2. SDA to Input (00)
                    // we just want to read the true state of the wire
                    hi2c.sda_port->MODER &= ~(0x3 << (hi2c.sda_pin * 2));

                    // make sure the SCL line is high
                    hi2c.scl_port->BSRR = (1 << hi2c.scl_pin);
                    // BSRR (Bit Set/Reset Register) allows to change the state of individual pins atomically (in a single CPU instruction cycle) without using a read-modify-write operation

                    // a small delay to give time electrical changes to occur on the bus
                    for (volatile uint32_t i = 0; i < 50; i++)
                        ;

                    // manually toggle SCL via GPIO to walk the slave's internal shift counter forward uint it releases SDA
                    for (uint8_t i = 0; i < 9; i++)
                    {
                        // check if the SDA is not released
                        if (hi2c.sda_port->IDR & (1 << hi2c.sda_pin))
                        {
                            // if released, exit early
                            bus_released = 1;
                            break;
                        }

                        // pull the SCL line low to step forward in the slave's internal state machine
                        hi2c.scl_port->BSRR = (1 << (hi2c.scl_pin + 16));

                        for (volatile uint32_t i = 0; i < 50; i++)
                            ;

                        // release the SCL line high
                        hi2c.scl_port->BSRR = (1 << (hi2c.scl_pin));

                        for (volatile uint32_t i = 0; i < 50; i++)
                            ;
                    }

                    if (bus_released)
                    {
                        // generate stop
                    }

                    // return the pins to AF
                }
            }
        }
        // if (BMP280_TriggerMeasurements(&hbmp) != BMP280_OK)
        // {
        //     return 1;
        // }

        // int32_t press_adc = 0;
        // int32_t temp_adc = 0;

        // if (BMP280_ReadMeasurements(&hbmp, &press_adc, &temp_adc) != BMP280_OK)
        // {
        //     return 1;
        // }

        // int32_t temp_value = 0;
        // uint32_t press_value = 0;
        // BMP280_CalculateData(&hbmp, press_adc, temp_adc, &temp_value, &press_value);

        // printf("Temp: %" PRId32 " degC | Press: %" PRIu32 " hPa\r\n", temp_value / 100, press_value / 256 / 100);
        // fflush(stdout);
    }

    return 0;
}