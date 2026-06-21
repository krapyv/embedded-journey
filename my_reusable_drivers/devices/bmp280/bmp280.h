#ifndef BMP280_H
#define BMP280_H

#include <stdint.h>

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
#define BMP280_REG_CALIB_START 0x88
// 0xA0 and 0xA1 are reserved
#define BMP280_REG_CALIB_LENGTH 24 // (0x9F - 0x88) + 1 = 0x17 + 1 = 23 + 1 = 24

#endif