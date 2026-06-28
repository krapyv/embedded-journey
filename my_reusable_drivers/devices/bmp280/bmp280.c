#include <stdio.h>
#include "bmp280.h"
#include "systick.h"

BMP280_Status_t BMP280_Calibration(BMP280_HandleTypeDef *hbmp)
{
    // early rejection of a transaction with invalid parameters
    if (hbmp == NULL || hbmp->hi2c == NULL)
    {
        return BMP280_ERROR;
    }

    // declare and initialize the arrays
    uint8_t pSend = BMP280_REG_CALIB_START;
    uint8_t pReceive[BMP280_REG_CALIB_LENGTH] = {0};

    // call the Transmit_Receive transaction function
    if (I2C_Master_Transmit_Receive(hbmp->hi2c, hbmp->slave_addr, &pSend, pReceive, 1, BMP280_REG_CALIB_LENGTH) != I2C_OK)
    {
        // if the transaction failed, exit the function
        return BMP280_ERROR;
    }

    // if the function succedded, we have 24 bytes of calibration data
    // now we need to reconstruct 12 compensation (calibration) parameters
    hbmp->calib.dig_T1 = ((uint16_t)pReceive[1] << 8) | pReceive[0];
    // if the pReceive[2], pReceive[4] etc (all the LSB parts) were int8_t and negative, sign extension would corrupt upper byte
    // since they are uint8_t, the unsigned promotion fills upper bits with zeros
    hbmp->calib.dig_T2 = ((int16_t)pReceive[3] << 8) | pReceive[2];
    hbmp->calib.dig_T3 = ((int16_t)pReceive[5] << 8) | pReceive[4];
    hbmp->calib.dig_P1 = ((uint16_t)pReceive[7] << 8) | pReceive[6];
    hbmp->calib.dig_P2 = ((int16_t)pReceive[9] << 8) | pReceive[8];
    hbmp->calib.dig_P3 = ((int16_t)pReceive[11] << 8) | pReceive[10];
    hbmp->calib.dig_P4 = ((int16_t)pReceive[13] << 8) | pReceive[12];
    hbmp->calib.dig_P5 = ((int16_t)pReceive[15] << 8) | pReceive[14];
    hbmp->calib.dig_P6 = ((int16_t)pReceive[17] << 8) | pReceive[16];
    hbmp->calib.dig_P7 = ((int16_t)pReceive[19] << 8) | pReceive[18];
    hbmp->calib.dig_P8 = ((int16_t)pReceive[21] << 8) | pReceive[20];
    hbmp->calib.dig_P9 = ((int16_t)pReceive[23] << 8) | pReceive[22];

    return BMP280_OK;
}

BMP280_Status_t BMP280_TriggerMeasurements(BMP280_HandleTypeDef *hbmp)
{
    // early rejection of a transaction with invalid parameters
    if (hbmp == NULL || hbmp->hi2c == NULL)
    {
        return BMP280_ERROR;
    }

    // configuring the "ctrl_meas" register
    uint8_t reconstructed_meas = ((uint8_t)hbmp->ctrl_meas.osrs_t << 5) | ((uint8_t)hbmp->ctrl_meas.osrs_p << 2) | (hbmp->ctrl_meas.mode);

    // two-byte transmit buffer
    const uint8_t transmit_length = 2;
    uint8_t transmit[transmit_length] = {
        (uint8_t)BMP280_REG_CTRL_MEAS, reconstructed_meas};

    // I2C write to 0xF4
    if (I2C_Master_Transmit(hbmp->hi2c, hbmp->slave_addr, transmit, transmit_length) != I2C_OK)
    {
        return BMP280_ERR_CONFIG;
    }

    // polling the bit 3 register 0xF3 "status" to track whether the transaction is ongoing or finished or error

    // capture the start timestamp
    uint32_t start = SysTick_GetTick();
    uint8_t pSend = (uint8_t)BMP280_REG_STATUS;
    uint8_t status_reg = 0;

    // poll the bit 3 of the register "status"

    do
    {
        if (I2C_Master_Transmit_Receive(hbmp->hi2c, hbmp->slave_addr, &pSend, &status_reg, 1, 1) != I2C_OK)
        {
            return BMP280_ERROR;
        }
        if (SysTick_GetTick() - start >= 15U)
        {
            return BMP280_ERR_TIMEOUT;
        }
    } while (status_reg & (1 << 3));

    return BMP280_OK;
}

BMP280_Status_t BMP280_ReadMeasurements(BMP280_HandleTypeDef *hbmp, int32_t *press_adc, int32_t *temp_adc)
{
    // early rejection of a transaction with invalid parameters
    if (hbmp == NULL || hbmp->hi2c == NULL)
    {
        return BMP280_ERROR;
    }

    // burst read of raw adc measurement values
    // 6 bytes - 3 of pressure, 3 of temperature
    const uint8_t burst_read_length = 6;
    uint8_t raw_adc[burst_read_length] = {0};

    uint8_t burst_start = (uint8_t)BMP280_REG_PRESS_MSB;

    if (I2C_Master_Transmit_Receive(hbmp->hi2c, hbmp->slave_addr, &burst_start, raw_adc, 1, 6) != I2C_OK)
    {
        return BMP280_ERROR;
    }

    // after the transmission we have 6 bytes:
    // raw_adc[0] - 0xF7 - MSB of pressure
    // raw_adc[1] - 0xF8 - LSB of pressure
    // raw_adc[2] - 0xF9 (bits 7, 6, 5, 4) - XLSB of pressure
    // raw_adc[3] - 0xFA - MSB of temperature
    // raw_adc[4] - 0xFB - LSB of temperature
    // raw_adc[5] - 0xFC (bits 7, 6, 5, 4) - XLSB of temperature

    // pressure adc reconstruction
    *press_adc = (int32_t)((uint32_t)raw_adc[0] << 12) | ((uint32_t)raw_adc[1] << 4) | ((uint32_t)raw_adc[2] >> 4);

    // temperature adc reconstruction
    *temp_adc = (int32_t)((uint32_t)raw_adc[3] << 12) | ((uint32_t)raw_adc[4] << 4) | ((uint32_t)raw_adc[5] >> 4);

    return BMP280_OK;
}

// returns temperature in DegC, resolution is 0.01 DegC (5123 equals 51.23 Degrees)
int32_t BMP280_Temp_Compensate(BMP280_HandleTypeDef *hbmp, int32_t temp_adc, int32_t *t_fine)
{
    int32_t var1, var2, T;
    var1 = (((temp_adc >> 3) - ((int32_t)hbmp->calib.dig_T1 << 1)) * ((int32_t)hbmp->calib.dig_T2)) >> 11;
    var2 = (((((temp_adc >> 4) - ((int32_t)hbmp->calib.dig_T1)) * ((temp_adc >> 4) - ((int32_t)hbmp->calib.dig_T1))) >> 12) * ((int32_t)hbmp->calib.dig_T3)) >> 14;

    *t_fine = var1 + var2;
    T = (*t_fine * 5 + 128) >> 8;

    return T;
}

// returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits).
// 24674867 = 24674867/256 = 96386.2 Pa = 963.862 hPa
uint32_t BMP280_Pressure_Compensate(BMP280_HandleTypeDef *hbmp, int32_t press_adc, int32_t t_fine)
{
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)hbmp->calib.dig_P6;
    var2 = var2 + ((var1 * (int64_t)hbmp->calib.dig_P5) << 17);
    var2 = var2 + (((int64_t)hbmp->calib.dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)hbmp->calib.dig_P3) >> 8) + ((var1 * (int64_t)hbmp->calib.dig_P2) << 12);
    var1 = ((((int64_t)1) << 47) + var1) * ((int64_t)hbmp->calib.dig_P1) >> 33;

    if (var1 == 0)
    {
        return 0; // avoid exception caused by division by zero
    }

    p = 1048576 - press_adc;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)hbmp->calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)hbmp->calib.dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)hbmp->calib.dig_P7) << 4);

    return (uint32_t)p;
}

// high-lever function that manages compensation calculations
void BMP280_CalculateData(BMP280_HandleTypeDef *hbmp, int32_t press_adc, int32_t temp_adc, int32_t *temp, uint32_t *press)
{
    int32_t t_fine = 0;

    *temp = BMP280_Temp_Compensate(hbmp, temp_adc, &t_fine);

    *press = BMP280_Pressure_Compensate(hbmp, press_adc, t_fine);

    return;
}