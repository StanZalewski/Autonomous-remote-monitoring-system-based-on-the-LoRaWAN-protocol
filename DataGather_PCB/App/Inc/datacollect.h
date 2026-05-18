#ifndef DATACOLLECT_H
#define DATACOLLECT_H

#include "stm32g0xx_hal.h"

// SHT45 data
extern float temperature_sht45;
extern float humidity_sht45;

// VEML7700 data
extern float lux_veml7700;
extern float whiteRatio_veml7700;

// BMP390 data
extern float temperature_bmp390;
extern float pressure_bmp390;
extern float bmp390_t_lin;

void CollectData(void);

#endif