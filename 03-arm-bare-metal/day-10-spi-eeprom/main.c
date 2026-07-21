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
    hi2c.sb_hits = 0;
    hi2c.stop_hits = 0;
    hi2c.start_pending_hits = 0;

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

    hbmp.state = BMP280_STATE_INIT;

    // test loop
    while (1)
    {
        I2C_Process();

        switch (hbmp.state)
        {
        case BMP280_STATE_IDLE:
            break;
        case BMP280_STATE_INIT:
            if (BMP280_Init(&hbmp, meas) != BMP280_OK)
            {
                hbmp.state = BMP280_STATE_ERROR;
            }
            break;
        case BMP280_STATE_READ_CALIBRATION:
            if (BMP280_ReadCalibration(&hbmp) != BMP280_OK)
            {
                hbmp.state = BMP280_STATE_ERROR;
            }
            break;
        case BMP280_STATE_RECONSTRUCT_CALIBRATION:
            BMP280_ReconstructCalibration(&hbmp);
            break;
        case BMP280_STATE_CTRL_MEAS:
            if (BMP280_WriteCtrlMeas(&hbmp) != BMP280_OK)
            {
                hbmp.state = BMP280_STATE_ERROR;
            }
            break;
        case BMP280_STATE_MEASURING:
            if (BMP280_Measuring(&hbmp) != BMP280_OK)
            {
                hbmp.state = BMP280_STATE_ERROR;
            }
            break;
        case BMP280_STATE_READ_MEASURAMENTS:
            if (BMP280_ReadMeasurements(&hbmp) != BMP280_OK)
            {
                hbmp.state = BMP280_STATE_ERROR;
            }
            break;
        case BMP280_STATE_RECONSTRUCT_MEASURAMENTS:
            BMP280_ReconstructMeasurements(&hbmp);
            break;
        case BMP280_STATE_COMPENSATE:
            BMP280_CalculateData(&hbmp);
            break;
        case BMP280_STATE_READY:
            printf("Temp: %" PRId32 " degC | Press: %" PRIu32 " hPa\r\n", hbmp.temp_value / 100, hbmp.press_value / 256 / 100);
            fflush(stdout);

            // printf("start_pending_hits: %d | sb_hits: %d\r\n", hi2c.start_pending_hits, hi2c.sb_hits);

            hbmp.request_status = BMP280_REQUEST_NONE;
            hbmp.retries = 0;
            hbmp.measure_start_tick = 0;
            hbmp.measure_start_tick_status = BMP280_START_TICK_NEVER_CAPTURED;
            // BMP start measurements
            hbmp.state = BMP280_STATE_CTRL_MEAS;

            break;
        case BMP280_STATE_ERROR:
            if (hbmp.retries >= 3)
            {
                // the counter is exhausted
                hbmp.state = BMP280_STATE_FAULT;
                break;
            }

            if (hbmp.hi2c->state == I2C_STATE_IDLE)
            {
                hbmp.retries++;

                // begin a transaction from the beginning (Calibration is read once at the very beginning, so omit the state)
                hbmp.state = BMP280_STATE_CTRL_MEAS;
            }

            break;
        case BMP280_STATE_FAULT:
            printf("The BMP280 sensor experienced hard fault!");
            fflush(stdout);
            break;
        }
    }
    return 0;
}