#ifndef BMP280_H
#define BMP280_H

#include <stdint.h>
#include "i2c.h"

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
} BMP280_HandleTypeDef;

#endif