// #include <stdio.h>
// #include <inttypes.h>
// #include "systick.h"
// #include "bmp280.h"
// #include "i2c.h"
// #include "uart.h"

// // globally declared vaiable with physically allocated memory in RAM
// I2C_HandleTypeDef hi2c;

// // TODO: reusable SWRST-precondition function, the refusal contract, and HARD_FAULT
// int main(void)
// {
//     // define and initialize structs
//     hi2c.channel = I2C_CHANNEL_1;
//     hi2c.scl_port = GPIOB;
//     hi2c.scl_pin = 6;
//     hi2c.sda_port = GPIOB;
//     hi2c.sda_pin = 7;
//     hi2c.state = I2C_STATE_IDLE;

//     BMP280_HandleTypeDef hbmp = {
//         .hi2c = &hi2c,
//         .slave_addr = BMP280_I2C_ADDR,
//         .isInitialized = 0};

//     BMP280_Ctrl_Meas_t meas = {
//         .osrs_p = BMP280_OSRS_P_OVRSMP_1,
//         .osrs_t = BMP280_OSRS_T_OVRSMP_1,
//         .mode = BMP280_FORCED_MODE};

//     // peripherals init
//     I2C_Init();
//     SysTick_Init((uint32_t)SYSTICK_FREQUENCY_16MHZ);

//     uint8_t isUsartInitialized = 0;

//     hbmp.state = BMP280_STATE_INIT;

//     // test loop
//     while (1)
//     {
//         I2C_Process();

//         BMP280_Poll(&hbmp, meas);

//         if (hbmp.isInitialized)
//         {
//             if (!isUsartInitialized)
//             {
//                 usart2_init();
//                 isUsartInitialized = 1;
//             }

//             if (hbmp.state == BMP280_STATE_READY)
//             {
//                 printf("Temp: %" PRId32 " degC | Press: %" PRIu32 " hPa\r\n", hbmp.temp_value / 100, hbmp.press_value / 256 / 100);
//                 fflush(stdout);

//                 // BMP start measurements
//                 hbmp.state = BMP280_STATE_CTRL_MEAS;
//             }

//             if (hbmp.state == BMP280_STATE_FAULT)
//             {
//                 printf("The BMP280 sensor experienced hard fault!");
//                 fflush(stdout);
//             }
//         }
//     }

//     return 0;
// }

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
    hi2c.state = I2C_STATE_IDLE;

    BMP280_HandleTypeDef hbmp = {
        .hi2c = &hi2c,
        .slave_addr = BMP280_I2C_ADDR,
        .isInitialized = 0};

    BMP280_Ctrl_Meas_t meas = {
        .osrs_p = BMP280_OSRS_P_OVRSMP_1,
        .osrs_t = BMP280_OSRS_T_OVRSMP_1,
        .mode = BMP280_FORCED_MODE};

    // peripherals init
    I2C_Init();
    SysTick_Init((uint32_t)SYSTICK_FREQUENCY_16MHZ);
    usart2_init();
    uint8_t isUsartInitialized = 0;

    hbmp.state = BMP280_STATE_INIT;

    uint32_t counter = 0;
    // test loop
    while (1)
    {
        I2C_Process();
        // poll the bit 3 of the register "status"

        BMP280_Poll(&hbmp, meas);

        while (hi2c.state != I2C_STATE_DONE)
        {
            I2C_Process();
        }

        if (hbmp.state == BMP280_STATE_READY)
        {
            printf("Temp and press \r\n");
            fflush(stdout);

            // BMP start measurements
            hbmp.state = BMP280_STATE_CTRL_MEAS;
        }
    }

    return 0;
}