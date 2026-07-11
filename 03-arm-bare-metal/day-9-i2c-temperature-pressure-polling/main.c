#include <stdio.h>
#include <inttypes.h>
#include "systick.h"
#include "bmp280.h"
#include "i2c.h"
#include "uart.h"

// globally declared vaiable with physically allocated memory in RAM
I2C_HandleTypeDef hi2c;

// TODO: reusable SWRST-precondition function, the refusal contract, and HARD_FAULT
int main(void)
{
    // define and initialize structs
    hi2c.channel = I2C_CHANNEL_1;
    hi2c.scl_port = GPIOB;
    hi2c.scl_pin = 6;
    hi2c.sda_port = GPIOB;
    hi2c.sda_pin = 7;

    BMP280_HandleTypeDef hbmp = {
        .hi2c = &hi2c,
        .slave_addr = BMP280_I2C_ADDR};

    BMP280_Ctrl_Meas_t meas = {
        .osrs_p = BMP280_OSRS_P_OVRSMP_1,
        .osrs_t = BMP280_OSRS_T_OVRSMP_1,
        .mode = BMP280_FORCED_MODE};

    // peripherals init
    I2C_Init();
    SysTick_Init((uint32_t)SYSTICK_FREQUENCY_16MHZ);
    if (BMP280_Init(&hbmp, meas) != BMP280_OK)
    {
        return 1;
    }
    usart2_init();

    // test loop
    while (1)
    {

        I2C_Process();
        if (BMP280_TriggerMeasurements(&hbmp) != BMP280_OK)
        {
            return 1;
        }

        int32_t press_adc = 0;
        int32_t temp_adc = 0;

        if (BMP280_ReadMeasurements(&hbmp, &press_adc, &temp_adc) != BMP280_OK)
        {
            return 1;
        }

        int32_t temp_value = 0;
        uint32_t press_value = 0;
        BMP280_CalculateData(&hbmp, press_adc, temp_adc, &temp_value, &press_value);

        printf("Temp: %" PRId32 " degC | Press: %" PRIu32 " hPa\r\n", temp_value / 100, press_value / 256 / 100);
        fflush(stdout);
    }

    return 0;
}