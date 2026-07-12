#include <stdio.h>
#include "bmp280.h"
#include "systick.h"

BMP280_Status_t BMP280_ReadCalibration(BMP280_HandleTypeDef *hbmp)
{
    // early rejection of a transaction with invalid parameters
    if (hbmp == NULL || hbmp->hi2c == NULL)
    {
        return BMP280_ERROR;
    }

    if (hbmp->request_status == BMP280_REQUEST_FIRED)
    {
        if (hbmp->hi2c->state == I2C_STATE_DONE)
        {
            hbmp->request_status = BMP280_REQUEST_NONE;
            hbmp->state = BMP280_STATE_RECONSTRUCT_CALIBRATION;
        }

        // if the transaction is not done, exit without advancing the state
        return BMP280_OK;
    }

    // declare and initialize the arrays
    hbmp->register_addr = BMP280_REG_CALIB_START;

    // call the Transmit_Receive transaction function
    if (I2C_Master_Transmit_Receive(hbmp->slave_addr, &(hbmp->register_addr), &(hbmp->raw_calib), 1U, BMP280_REG_CALIB_LENGTH) != I2C_OK)
    {
        // if the transaction failed, exit the function
        return BMP280_ERROR;
    }

    hbmp->state = BMP280_REQUEST_FIRED;

    return BMP280_OK;
}

void BMP280_ReconstructCalibration(BMP280_HandleTypeDef *hbmp)
{
    // if the function succedded, we have 24 bytes of calibration data
    // now we need to reconstruct 12 compensation (calibration) parameters
    hbmp->calib.dig_T1 = ((uint16_t)(hbmp->raw_calib[1]) << 8) | hbmp->raw_calib[0];
    // if the pReceive[2], pReceive[4] etc (all the LSB parts) were int8_t and negative, sign extension would corrupt upper byte
    // since they are uint8_t, the unsigned promotion fills upper bits with zeros
    hbmp->calib.dig_T2 = ((int16_t)(hbmp->raw_calib[3]) << 8) | hbmp->raw_calib[2];
    hbmp->calib.dig_T3 = ((int16_t)(hbmp->raw_calib[5]) << 8) | hbmp->raw_calib[4];
    hbmp->calib.dig_P1 = ((uint16_t)(hbmp->raw_calib[7]) << 8) | hbmp->raw_calib[6];
    hbmp->calib.dig_P2 = ((int16_t)(hbmp->raw_calib[9]) << 8) | hbmp->raw_calib[8];
    hbmp->calib.dig_P3 = ((int16_t)(hbmp->raw_calib[11]) << 8) | hbmp->raw_calib[10];
    hbmp->calib.dig_P4 = ((int16_t)(hbmp->raw_calib[13]) << 8) | hbmp->raw_calib[12];
    hbmp->calib.dig_P5 = ((int16_t)(hbmp->raw_calib[15]) << 8) | hbmp->raw_calib[14];
    hbmp->calib.dig_P6 = ((int16_t)(hbmp->raw_calib[17]) << 8) | hbmp->raw_calib[16];
    hbmp->calib.dig_P7 = ((int16_t)(hbmp->raw_calib[19]) << 8) | hbmp->raw_calib[18];
    hbmp->calib.dig_P8 = ((int16_t)(hbmp->raw_calib[21]) << 8) | hbmp->raw_calib[20];
    hbmp->calib.dig_P9 = ((int16_t)(hbmp->raw_calib[23]) << 8) | hbmp->raw_calib[22];

    // the BMP280 is fully initialized
    hbmp->isInitialized = 1;

    hbmp->state = BMP280_STATE_CTRL_MEAS;
    return;
}

BMP280_Status_t BMP280_WriteCtrlMeas(BMP280_HandleTypeDef *hbmp)
{
    if (hbmp == NULL || hbmp->hi2c == NULL)
    {
        return BMP280_ERROR;
    }

    if (hbmp->request_status == BMP280_REQUEST_FIRED)
    {
        if (hbmp->hi2c->state == I2C_STATE_DONE)
        {
            hbmp->request_status = BMP280_REQUEST_NONE;
            hbmp->state = BMP280_STATE_MEASURING;
        }

        // if the transaction is not done, exit without advancing the state
        return BMP280_OK;
    }

    // configuring the "ctrl_meas" register
    uint8_t reconstructed_meas = ((uint8_t)hbmp->ctrl_meas.osrs_t << 5) | ((uint8_t)hbmp->ctrl_meas.osrs_p << 2) | (hbmp->ctrl_meas.mode);

    // two-byte transmit buffer
    const uint8_t transmit_length = 2;
    uint8_t transmit[2] = {(uint8_t)BMP280_REG_CTRL_MEAS, reconstructed_meas};

    // I2C write to 0xF4
    if (I2C_Master_Transmit(hbmp->slave_addr, transmit, transmit_length) != I2C_OK)
    {
        return BMP280_ERROR; // parameters are null or there are errors on the bus during BUSY waiting
    }

    hbmp->request_status = BMP280_REQUEST_FIRED;

    return BMP280_OK;
}

BMP280_Status_t BMP280_Measuring(BMP280_HandleTypeDef *hbmp)
{
    // early rejection of a transaction with invalid parameters
    if (hbmp == NULL || hbmp->hi2c == NULL)
    {
        return BMP280_ERROR;
    }

    // polling the bit 3 register 0xF3 "status" to track whether the transaction is ongoing or finished or error

    // capture the start timestamp
    if (hbmp->measure_start_tick_status == BMP280_START_TICK_NEVER_CAPTURED)
    {
        hbmp->measure_start_tick = SysTick_GetTick();
        hbmp->measure_start_tick_status = BMP280_START_TICK_ALREADY_CAPTURED;
    }

    if (hbmp->request_status == BMP280_REQUEST_FIRED)
    {
        if (hbmp->hi2c->state == I2C_STATE_DONE)
        {
            hbmp->request_status = BMP280_REQUEST_NONE;

            // if the transaction is over and the measurement is finished (bit 3 is 0)
            if (!(hbmp->status_reg_state & (1 << 3)))
            {
                hbmp->state = BMP280_STATE_READ_MEASURAMENTS;
            }

            // the current transaction is done, so check for timeout
            if (SysTick_GetTick() - hbmp->measure_start_tick >= 15U)
            {
                // reset the measurement start tick status since for this measurement it is over
                hbmp->measure_start_tick_status = BMP280_START_TICK_NEVER_CAPTURED;
                hbmp->state = BMP280_STATE_ERROR;
                return BMP280_ERR_TIMEOUT;
            }
            // if the transaction is over but the measurement is still ongoing (bit 3 is still 1)
            // not advance to the next state, re-entry the function next cycle and start the transaction again

            return BMP280_OK;
        }
        else
        {
            // if the transaction is not done, exit
            return BMP280_OK;
        }
    }

    hbmp->register_addr = (uint8_t)BMP280_REG_STATUS;

    // poll the bit 3 of the register "status"

    if (I2C_Master_Transmit_Receive(hbmp->slave_addr, &(hbmp->register_addr), &(hbmp->status_reg_state), 1U, 1U) != I2C_OK)
    {
        return BMP280_ERROR;
    }

    hbmp->request_status = BMP280_REQUEST_FIRED;

    return BMP280_OK;
}

BMP280_Status_t BMP280_ReadMeasurements(BMP280_HandleTypeDef *hbmp)
{
    // early rejection of a transaction with invalid parameters
    if (hbmp == NULL || hbmp->hi2c == NULL)
    {
        return BMP280_ERROR;
    }

    if (hbmp->request_status == BMP280_REQUEST_FIRED)
    {
        if (hbmp->hi2c->state == I2C_STATE_DONE)
        {
            hbmp->request_status = BMP280_REQUEST_NONE;
            hbmp->state = BMP280_STATE_RECONSTRUCT_MEASURAMENTS;
        }

        // if the transaction is not done, exit without advancing the state
        return BMP280_OK;
    }

    // burst read of raw adc measurement values
    // 6 bytes - 3 of pressure, 3 of temperature
    hbmp->register_addr = (uint8_t)BMP280_REG_PRESS_MSB;

    if (I2C_Master_Transmit_Receive(hbmp->slave_addr, &(hbmp->register_addr), hbmp->raw_adc, 1U, 6U) != I2C_OK)
    {
        return BMP280_ERROR;
    }

    hbmp->request_status = BMP280_REQUEST_FIRED;

    return BMP280_OK;
}

void BMP280_ReconstructMeasurements(BMP280_HandleTypeDef *hbmp)
{
    // after the transmission we have 6 bytes:
    // raw_adc[0] - 0xF7 - MSB of pressure
    // raw_adc[1] - 0xF8 - LSB of pressure
    // raw_adc[2] - 0xF9 (bits 7, 6, 5, 4) - XLSB of pressure
    // raw_adc[3] - 0xFA - MSB of temperature
    // raw_adc[4] - 0xFB - LSB of temperature
    // raw_adc[5] - 0xFC (bits 7, 6, 5, 4) - XLSB of temperature

    // pressure adc reconstruction
    hbmp->press_adc = (int32_t)((uint32_t)(hbmp->raw_adc[0]) << 12) | ((uint32_t)(hbmp->raw_adc[1]) << 4) | ((uint32_t)(hbmp->raw_adc[2]) >> 4);

    // temperature adc reconstruction
    hbmp->temp_adc = (int32_t)((uint32_t)(hbmp->raw_adc[3]) << 12) | ((uint32_t)(hbmp->raw_adc[4]) << 4) | ((uint32_t)(hbmp->raw_adc[5]) >> 4);

    hbmp->state = BMP280_STATE_COMPENSATE;
    return;
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
void BMP280_CalculateData(BMP280_HandleTypeDef *hbmp)
{
    int32_t t_fine = 0;

    hbmp->temp_value = BMP280_Temp_Compensate(hbmp, hbmp->temp_adc, &t_fine);

    hbmp->press_value = BMP280_Pressure_Compensate(hbmp, hbmp->press_adc, t_fine);

    hbmp->state = BMP280_STATE_READY;

    return;
}

BMP280_Status_t BMP280_Init(BMP280_HandleTypeDef *hbmp, BMP280_Ctrl_Meas_t meas)
{
    if (hbmp == NULL || hbmp->hi2c == NULL)
    {
        return BMP280_ERROR;
    }

    if (hbmp->request_status == BMP280_REQUEST_FIRED)
    {
        if (hbmp->hi2c->state == I2C_STATE_DONE)
        {
            hbmp->request_status = BMP280_REQUEST_NONE;

            // if received_id is not 0x58, then we have wrong device on the bus or a dead bus (floating SDA line returns 0xFF)
            if (hbmp->requested_chip_id != (uint8_t)BMP280_CHIP_ID)
            {
                return BMP280_ERR_CHIP_ID;
            }

            // if the received_id corresponds with our anticipated result, populate ctrl_meas inside the handle
            hbmp->ctrl_meas = meas;

            hbmp->state = BMP280_STATE_READ_CALIBRATION;
        }

        // if the transaction is not done, exit without advancing the state
        return BMP280_OK;
    }

    hbmp->register_addr = (uint8_t)BMP280_REG_ID;

    if (I2C_Master_Transmit_Receive(hbmp->slave_addr, &(hbmp->register_addr), &(hbmp->requested_chip_id), 1U, 1U) != I2C_OK)
    {
        return BMP280_ERROR;
    }

    return BMP280_OK;
}

void BMP280_Poll(BMP280_HandleTypeDef *hbmp, BMP280_Ctrl_Meas_t meas)
{
    switch (hbmp->state)
    {
    case BMP280_STATE_IDLE:
        break;
    case BMP280_STATE_INIT:
        if (BMP280_Init(hbmp, meas) != BMP280_OK)
        {
            hbmp->state = BMP280_STATE_ERROR;
        }
        break;
    case BMP280_STATE_READ_CALIBRATION:
        if (BMP280_ReadCalibration(hbmp) != BMP280_OK)
        {
            hbmp->state = BMP280_STATE_ERROR;
        }
        break;
    case BMP280_STATE_RECONSTRUCT_CALIBRATION:
        BMP280_ReconstructCalibration(hbmp);
        break;
    case BMP280_STATE_CTRL_MEAS:
        if (BMP280_WriteCtrlMeas(hbmp) != BMP280_OK)
        {
            hbmp->state = BMP280_STATE_ERROR;
        }
        break;
    case BMP280_STATE_MEASURING:
        if (BMP280_Measuring(hbmp) != BMP280_OK)
        {
            hbmp->state = BMP280_STATE_ERROR;
        }
        break;
    case BMP280_STATE_READ_MEASURAMENTS:
        if (BMP280_ReadMeasurements(hbmp) != BMP280_OK)
        {
            hbmp->state = BMP280_STATE_ERROR;
        }
        break;
    case BMP280_STATE_RECONSTRUCT_MEASURAMENTS:
        BMP280_ReconstructMeasurements(hbmp);
        break;
    case BMP280_STATE_COMPENSATE:
        BMP280_CalculateData(hbmp);
        break;
    case BMP280_STATE_READY:
        hbmp->request_status = BMP280_REQUEST_NONE;
        hbmp->retries = 0;
        hbmp->measure_start_tick = 0;
        hbmp->measure_start_tick_status = BMP280_START_TICK_NEVER_CAPTURED;
        break;
    case BMP280_STATE_ERROR:
        if (hbmp->retries >= 3)
        {
            // the counter is exhausted
            hbmp->state = BMP280_STATE_FAULT;
            break;
        }

        if (hbmp->hi2c->state == I2C_STATE_IDLE)
        {
            hbmp->retries++;

            // begin a transaction from the beginning (Calibration is read once at the very beginning, so omit the state)
            hbmp->state = BMP280_STATE_CTRL_MEAS;
        }

        break;
    case BMP280_STATE_FAULT:
        break;
    }
}