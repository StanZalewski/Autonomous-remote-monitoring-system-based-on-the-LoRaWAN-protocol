#ifndef SHT45_H
#define SHT45_H

#include "stm32g0xx_hal.h"
#include <stdint.h>

HAL_StatusTypeDef SHT45_ReadTempRH(I2C_HandleTypeDef *hi2c, float *temperature_degC, float *humidity_pRH);

#endif 