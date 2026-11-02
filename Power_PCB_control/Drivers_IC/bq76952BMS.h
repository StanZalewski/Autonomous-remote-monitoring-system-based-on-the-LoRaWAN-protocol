#ifndef BQ76952BMS_H
#define BQ76952BMS_H

#include "stm32g0xx_hal.h"
#include <stdint.h>

#define BQ76952_I2C_ADDRESS  0x08

#define BQ76952_REG_SUBCMD_LOW        0x3E
#define BQ76952_REG_SUBCMD_HIGH       0x3F
#define BQ76952_REG_TRANSFER_BUF      0x40
#define BQ76952_REG_CHECKSUM          0x60
#define BQ76952_REG_LENGTH            0x61

#define BQ76952_SUBCMD_UNSEAL_1       0x0414
#define BQ76952_SUBCMD_UNSEAL_2       0x3672
#define BQ76952_SUBCMD_SET_CFGUPDATE  0x0090
#define BQ76952_SUBCMD_EXIT_CFGUPDATE 0x0092

#define BQ76952_REG_CONTROL_STATUS    0x00
#define BQ76952_REG_BATTERY_STATUS    0x12

HAL_StatusTypeDef BQ76952_ReadRegister(I2C_HandleTypeDef *hi2c, uint8_t reg_addr, uint8_t *data, uint8_t bytes);
HAL_StatusTypeDef BQ76952_WriteRegister(I2C_HandleTypeDef *hi2c, uint8_t reg_addr, uint8_t *data, uint8_t bytes);
HAL_StatusTypeDef BQ76952_WriteRAM(I2C_HandleTypeDef *hi2c, uint16_t ram_addr, uint8_t *data, uint8_t length);
HAL_StatusTypeDef BQ76952_ReadRAM(I2C_HandleTypeDef *hi2c, uint16_t ram_addr, uint8_t *data, uint8_t length);
HAL_StatusTypeDef BQ76952_EnterConfigUpdateMode(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef BQ76952_ExitConfigUpdate(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef BQ76952_InitBasicConfig(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef BQ76952_InitProtectionSettings(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef BQ76952_InitProtectionValues(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef BQ76952_InitFETAndPins(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef BQ76952_InitCellBalancing(I2C_HandleTypeDef *hi2c);

HAL_StatusTypeDef BQ76952_VerifyCritical(I2C_HandleTypeDef *hi2c);
#endif