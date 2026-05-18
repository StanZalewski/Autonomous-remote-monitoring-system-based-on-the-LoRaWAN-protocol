#ifndef VEML7700_H
#define VEML7700_H
#include "stm32g0xx_hal.h"


typedef enum {
    ALS_GAIN_1   = 0,   
    ALS_GAIN_2   = 1,
    ALS_GAIN_1_8 = 2,
    ALS_GAIN_1_4 = 3,
} measurement_gain;

typedef enum {
    T_25_MS  = 12,   
    T_50_MS  = 8,
    T_100_MS = 0,
    T_200_MS = 1,
    T_400_MS = 2,
    T_800_MS = 3,
} integration_time_setting;

typedef enum {
    ONE_READING    = 0,   
    TWO_READINGS   = 1,
    FOUR_READINGS  = 2,
    EIGHT_READINGS = 3,
}persistence_protect_number;

typedef enum {
    DIS = 0,   
    EN  = 1,
}en_dis;

typedef enum {
    POWER_ON  = 0,   
    SHUTDOWN = 1,
}power_state;

HAL_StatusTypeDef VEML7700_SetupParameters(I2C_HandleTypeDef *hi2c, measurement_gain gain, integration_time_setting integrationTime, persistence_protect_number ppn, en_dis interruptEnable, power_state state);
HAL_StatusTypeDef VEML7700_ReadALS(I2C_HandleTypeDef *hi2c, uint16_t *als_counts, float *lux);
HAL_StatusTypeDef VEML7700_ReadWHITE(I2C_HandleTypeDef *hi2c, uint16_t *white_counts);

#endif