#include "bmp280.h"
#include "systick.h"

BMP280_Status_t BMP280_Calibration(BMP280_HandleTypeDef *hbmp)
{
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
    // to simplify the parameter passing
    I2C_HandleTypeDef *hi2c = hbmp->hi2c;
    uint8_t slave_addr = hbmp->slave_addr;

    // configuring the "ctrl_meas" register
    BMP280_Ctrl_Meas_t meas = hbmp->ctrl_meas;
    uint8_t reconstructed_meas = ((uint8_t)meas.osrs_t << 5) | ((uint8_t)meas.osrs_p << 2) | (meas.mode);

    // two-byte transmit buffer
    const uint8_t transmit_length = 2;
    uint8_t transmit[transmit_length] = {
        (uint8_t)BMP280_REG_CTRL_MEAS, reconstructed_meas};

    // I2C write to 0xF4
    if (I2C_Master_Transmit(hi2c, slave_addr, transmit, transmit_length) != I2C_OK)
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
        if (I2C_Master_Transmit_Receive(hi2c, slave_addr, &pSend, &status_reg, 1, 1) != I2C_OK)
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