#ifndef BMP280_H
#define BMP280_H

#include <stdint.h>
#include "i2c.h"

#define BMP280_CHIP_ID 0x58

#define BMP280_REG_ID 0xD0
#define BMP280_REG_RESET 0xE0
#define BMP280_REG_STATUS 0xF3
#define BMP280_REG_CTRL_MEAS 0xF4
#define BMP280_REG_CONFIG 0xF5
#define BMP280_REG_PRESS_MSB 0xF7
#define BMP280_REG_PRESS_LSB 0xF8
#define BMP280_REG_PRESS_XLSB 0xF9
#define BMP280_REG_TEMP_MSB 0xFA
#define BMP280_REG_TEMP_LSB 0xFB
#define BMP280_REG_TEMP_XLSB 0xFC

// Register of calibration data
#define BMP280_REG_CALIB_START (uint8_t)0x88
// 0xA0 and 0xA1 are reserved
#define BMP280_REG_CALIB_LENGTH 24 // (0x9F - 0x88) + 1 = 0x17 + 1 = 23 + 1 = 24

// Slave address variants
#define BMP280_I2C_ADDR 0x76     // SDO-low variant
#define BMP280_I2C_ADDR_ALT 0x77 // SDO-high variant

typedef enum
{
    BMP280_OK = 0,
    BMP280_ERROR,
    BMP280_ERR_TIMEOUT,
    BMP280_ERR_CHIP_ID,
    BMP280_ERR_CONFIG
} BMP280_Status_t;

typedef enum
{
    BMP280_OSRS_P_SKIPPED = 0, // 000
    BMP280_OSRS_P_OVRSMP_1,    // 001
    BMP280_OSRS_P_OVRSMP_2,    // 010
    BMP280_OSRS_P_OVRSMP_4,    // 011
    BMP280_OSRS_P_OVRSMP_8,    // 100
    BMP280_OSRS_P_OVRSMP_16,   // 101
} BMP280_OSRS_P_t;

typedef enum
{
    BMP280_OSRS_T_SKIPPED = 0, // 000
    BMP280_OSRS_T_OVRSMP_1,    // 001
    BMP280_OSRS_T_OVRSMP_2,    // 010
    BMP280_OSRS_T_OVRSMP_4,    // 011
    BMP280_OSRS_T_OVRSMP_8,    // 100
    BMP280_OSRS_T_OVRSMP_16,   // 101
} BMP280_OSRS_T_t;

typedef enum
{
    BMP280_SLEEP_MODE = 0,  // 00
    BMP280_FORCED_MODE = 1, // 01
    BMP280_NORMAL_MODE = 3  // 11
} BMP280_PWR_MODES_t;

typedef enum
{
    BMP280_REQUEST_NONE = 0,
    BMP280_REQUEST_FIRED = 1,
} BMP280_Request_Status_t;

typedef enum
{
    BMP280_START_TICK_NEVER_CAPTURED = 0,
    BMP280_START_TICK_ALREADY_CAPTURED = 1
} BMP280_Start_Tick_Status_t;

typedef enum
{
    BMP280_STATE_IDLE = 0,
    BMP280_STATE_REQUEST_CHIP_ID,
    BMP280_STATE_INIT,
    BMP280_STATE_READ_CALIBRATION,
    BMP280_STATE_RECONSTRUCT_CALIBRATION,
    BMP280_STATE_CTRL_MEAS,
    BMP280_STATE_MEASURING,
    BMP280_STATE_READ_MEASURAMENTS,
    BMP280_STATE_RECONSTRUCT_MEASURAMENTS,
    BMP280_STATE_COMPENSATE,
    BMP280_STATE_READY,
    BMP280_STATE_ERROR,
    BMP280_STATE_FAULT
} BMP280_State_t;

typedef struct
{
    uint16_t dig_T1; // 0x88/0x89
    int16_t dig_T2;  // 0x8A/0x8B
    int16_t dig_T3;  // 0x8C/0x8D
    uint16_t dig_P1; // 0x8E/0x8F
    int16_t dig_P2;  // 0x90/0x91
    int16_t dig_P3;  // 0x92/0x93
    int16_t dig_P4;  // 0x94/0x95
    int16_t dig_P5;  // 0x96/0x97
    int16_t dig_P6;  // 0x98/0x99
    int16_t dig_P7;  // 0x9A/0x9B
    int16_t dig_P8;  // 0x9C/0x9D
    int16_t dig_P9;  // 0x9E/0x9F
} BMP280_Calib_t;

typedef struct
{
    BMP280_OSRS_T_t osrs_t;
    BMP280_OSRS_P_t osrs_p;
    BMP280_PWR_MODES_t mode;
} BMP280_Ctrl_Meas_t;

typedef struct
{
    I2C_HandleTypeDef *hi2c;
    BMP280_Calib_t calib;
    BMP280_Ctrl_Meas_t ctrl_meas;
    uint8_t slave_addr;
    BMP280_State_t state;
    BMP280_Request_Status_t request_status;
    uint8_t retries;             // reset only on full-cycle success
    uint32_t measure_start_tick; // for measure status polling
    BMP280_Start_Tick_Status_t measure_start_tick_status;
    uint8_t register_addr;     // for register-address byte ahead of each Transmit_Receive
    uint8_t raw_calib[24];     // dedicated RX buffer for calibration burst
    uint8_t raw_adc[6];        // dedicated RX buffer for pressure/temp burst
    uint8_t ctrl_meas_tx[2];   // dedicated TX buffer for the CTRL_MEAS write
    uint8_t status_reg_state;  // dedicated RX byte for status register current state
    uint8_t requested_chip_id; // TX field for requested chip id

    int32_t press_adc;
    int32_t temp_adc;

    int32_t temp_value;
    uint32_t press_value;

    uint8_t isInitialized;
} BMP280_HandleTypeDef;

// function headers
BMP280_Status_t BMP280_ReadCalibration(BMP280_HandleTypeDef *hbmp);
void BMP280_ReconstructCalibration(BMP280_HandleTypeDef *hbmp);
BMP280_Status_t BMP280_WriteCtrlMeas(BMP280_HandleTypeDef *hbmp);
BMP280_Status_t BMP280_Measuring(BMP280_HandleTypeDef *hbmp);
BMP280_Status_t BMP280_ReadMeasurements(BMP280_HandleTypeDef *hbmp);
void BMP280_ReconstructMeasurements(BMP280_HandleTypeDef *hbmp);
int32_t BMP280_Temp_Compensate(BMP280_HandleTypeDef *hbmp, int32_t temp_adc, int32_t *t_fine);
uint32_t BMP280_Pressure_Compensate(BMP280_HandleTypeDef *hbmp, int32_t press_adc, int32_t t_fine);
void BMP280_CalculateData(BMP280_HandleTypeDef *hbmp);
BMP280_Status_t BMP280_Init(BMP280_HandleTypeDef *hbmp, BMP280_Ctrl_Meas_t meas);
void BMP280_Poll(BMP280_HandleTypeDef *hbmp, BMP280_Ctrl_Meas_t meas);

#endif
