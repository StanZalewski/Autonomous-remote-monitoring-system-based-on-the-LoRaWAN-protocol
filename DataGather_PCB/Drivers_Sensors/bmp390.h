#ifndef BMP390_H
#define BMP390_H

#include "stm32g0xx_hal.h"
#include <stdint.h>

#define BMP390_I2C_ADDR_SDO_GND    0x76
#define BMP390_REG_CHIP_ID         0x00
#define BMP390_REG_ERROR           0x02
#define BMP390_REG_STATUS          0x03
#define BMP390_REG_DATA_0          0x04  
#define BMP390_REG_DATA_1          0x05
#define BMP390_REG_DATA_2          0x06
#define BMP390_REG_DATA_3          0x07
#define BMP390_REG_DATA_4          0x08
#define BMP390_REG_DATA_5          0x09
#define BMP390_REG_PWR_CTRL        0x1B
#define BMP390_REG_OSR             0x1C
#define BMP390_REG_ODR             0x1D
#define BMP390_REG_CONFIG          0x1F
#define BMP390_REG_CALIB_DATA      0x31
#define BMP390_REG_CMD             0x7E

// Status bits
#define BMP390_STATUS_DRDY_PRESS   (1 << 5)
#define BMP390_STATUS_DRDY_TEMP    (1 << 6)

/**
 * @brief Calibration data structure
 * IMPORTANT: These are CONVERTED floating-point parameters, not raw NVM values!
 * The conversion from NVM to these values is done in BMP390_ReadCalibration()
 */
typedef struct {
    float par_t1;   // Converted from NVM
    float par_t2;   // Converted from NVM
    float par_t3;   // Converted from NVM
    float par_p1;   // Converted from NVM
    float par_p2;   // Converted from NVM
    float par_p3;   // Converted from NVM
    float par_p4;   // Converted from NVM
    float par_p5;   // Converted from NVM
    float par_p6;   // Converted from NVM
    float par_p7;   // Converted from NVM
    float par_p8;   // Converted from NVM
    float par_p9;   // Converted from NVM
    float par_p10;  // Converted from NVM
    float par_p11;  // Converted from NVM
    float t_lin;    // CRITICAL: Stores linearized temp for pressure compensation
} BMP390_CalibData;

HAL_StatusTypeDef BMP390_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef BMP390_ERROR_REGISTER(I2C_HandleTypeDef *hi2c, uint8_t *error_reg);
HAL_StatusTypeDef BMP390_CHIP_ID(I2C_HandleTypeDef *hi2c, uint8_t *chip_id);
HAL_StatusTypeDef BMP390_STATUS(I2C_HandleTypeDef *hi2c, uint8_t *status);
HAL_StatusTypeDef BMP390_ReadCalibration(I2C_HandleTypeDef *hi2c, BMP390_CalibData *calib);
HAL_StatusTypeDef BMP390_ReadRawTemperature(I2C_HandleTypeDef *hi2c, uint32_t *raw_temp);
float BMP390_CompensateTemperature(uint32_t raw_temp, BMP390_CalibData *calib);
HAL_StatusTypeDef BMP390_ReadRawPressure(I2C_HandleTypeDef *hi2c, uint32_t *raw_press);
float BMP390_CompensatePressure(uint32_t raw_press, BMP390_CalibData *calib);
HAL_StatusTypeDef BMP390_WaitForData(I2C_HandleTypeDef *hi2c, uint32_t timeout_ms);
#endif