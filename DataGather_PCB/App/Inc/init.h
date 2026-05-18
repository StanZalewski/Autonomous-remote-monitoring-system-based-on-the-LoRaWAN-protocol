#ifndef INIT_H
#define INIT_H

#include "stm32g0xx_hal.h"
#include "bmp390.h"

extern I2C_HandleTypeDef hi2c1;
extern BMP390_CalibData g_bmp390_calib;

void InitSensors();

#endif