#include "datacollect.h"
#include "init.h"
#include "sht45.h"
#include "veml7700.h"
#include "bmp390.h"
#include "stm32g0xx_hal.h"

extern I2C_HandleTypeDef hi2c1;

// SHT45 data
float temperature_sht45;
float humidity_sht45;

// VEML7700 data
float lux_veml7700;
float whiteRatio_veml7700;

// BMP390 data
float temperature_bmp390;
float pressure_bmp390;


void CollectData() {
    uint16_t als_counts;
    uint16_t white_counts;
    uint32_t raw_temp;
    uint32_t raw_press;

    // Collect SHT45 temperature and humidity data
    if (SHT45_ReadTempRH(&hi2c1, &temperature_sht45, &humidity_sht45) != HAL_OK) {
        temperature_sht45 = -999.0f;
        humidity_sht45 = -999.0f;
    }

    // Collect ALS (light) data
    if (VEML7700_ReadALS(&hi2c1, &als_counts, &lux_veml7700) != HAL_OK) {
        lux_veml7700 = -1.0f;
        als_counts = 0;
    }
    
    // Collect WHITE channel data
    if (VEML7700_ReadWHITE(&hi2c1, &white_counts) != HAL_OK) {
        white_counts = 0;
    }
    
    // Calculate WHITE/ALS ratio (avoid division by zero)
    if (als_counts > 0) {
        whiteRatio_veml7700 = (float)white_counts / (float)als_counts;
    } else {
        whiteRatio_veml7700 = 0.0f;
    }

    // BMP390 - Wait for data to be ready (timeout 100ms)
    if (BMP390_WaitForData(&hi2c1, 100) == HAL_OK) {
        
        // Read temperature first (this calculates and stores t_lin)
        if (BMP390_ReadRawTemperature(&hi2c1, &raw_temp) == HAL_OK) {
            temperature_bmp390 = BMP390_CompensateTemperature(raw_temp, &g_bmp390_calib);
        } else {
            temperature_bmp390 = -999.0f;
        }

        // Read pressure (uses t_lin from temperature compensation)
        if (BMP390_ReadRawPressure(&hi2c1, &raw_press) == HAL_OK) {
            pressure_bmp390 = BMP390_CompensatePressure(raw_press, &g_bmp390_calib);
        } else {
            pressure_bmp390 = -999.0f;
        }
    } else {
        // Timeout waiting for data
        temperature_bmp390 = -999.0f;
        pressure_bmp390 = -999.0f;
    }
}