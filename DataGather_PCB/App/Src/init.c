#include "datacollect.h"
#include "sht45.h"
#include "veml7700.h"
#include "bmp390.h"
#include "stm32g0xx_hal.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c1;

BMP390_CalibData g_bmp390_calib;


void InitSensors(){
    uint8_t error_reg;
    uint8_t chip_id;

    // Initialize VEML7700 light sensor
    if (VEML7700_SetupParameters(&hi2c1, ALS_GAIN_1_8, T_25_MS, TWO_READINGS, DIS, POWER_ON)!= HAL_OK){
        HAL_GPIO_WritePin(LED_STATE_GPIO_Port, LED_STATE_Pin, GPIO_PIN_RESET);
    }

    // Initialize BMP390 barometric pressure sensor
    // This performs: soft reset, chip ID check, configuration of OSR/ODR/filter, and enables normal mode
    if (BMP390_Init(&hi2c1) == HAL_OK) {
        // BMP390 initialized successfully
        
        // Read calibration data after initialization
        if (BMP390_ReadCalibration(&hi2c1, &g_bmp390_calib) == HAL_OK){
            g_bmp390_calib.t_lin = 0.0f;  // Initialize t_lin
        }
        
        // Optional: Check error register
        if (BMP390_ERROR_REGISTER(&hi2c1, &error_reg) == HAL_OK){
            // error_reg should be 0 if no errors
        }
        
        // Optional: Verify chip ID
        if (BMP390_CHIP_ID(&hi2c1, &chip_id) == HAL_OK){
            // chip_id should be 0x60
        }
    }
    else{
        HAL_GPIO_WritePin(LED_STATE_GPIO_Port, LED_STATE_Pin, GPIO_PIN_RESET);
    }
}