#include "veml7700.h"
#include "stm32g0xx_hal_i2c.h"

#define VEML7700_I2C_ADDR       0x10  // 7-bit address (will be shifted by HAL)
#define VEML7700_REG_CONFIG     0x00
#define VEML7700_REG_ALS_DATA   0x04
#define VEML7700_REG_WHITE_DATA 0x05

static float veml7700_resolution = 1.0752;


HAL_StatusTypeDef VEML7700_SetupParameters(I2C_HandleTypeDef *hi2c, measurement_gain gain, integration_time_setting integrationTime, persistence_protect_number ppn, en_dis interruptEnable, power_state state){
    uint16_t reg_value = ((gain & 0x03) << 11) | ((integrationTime & 0x0f) << 6) | ((ppn & 0x03) << 4) | ((interruptEnable & 0x01) << 1) | (state & 0x01);
    
    uint8_t data[2];
    data[0] = (reg_value & 0xFF);        // LSB
    data[1] = ((reg_value >> 8) & 0xFF); // MSB


    return HAL_I2C_Mem_Write(hi2c, (VEML7700_I2C_ADDR << 1), VEML7700_REG_CONFIG, I2C_MEMADD_SIZE_8BIT, data, 2, HAL_MAX_DELAY);
}


HAL_StatusTypeDef VEML7700_ReadALS(I2C_HandleTypeDef *hi2c, uint16_t *als_counts, float *lux) {
    HAL_StatusTypeDef status;
    uint8_t data[2];
    
    // Read 2 bytes from ALS register (simpler!)
    status = HAL_I2C_Mem_Read(hi2c, (VEML7700_I2C_ADDR << 1), VEML7700_REG_ALS_DATA, I2C_MEMADD_SIZE_8BIT, data, 2, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return status;
    }
    
    
    // Combine bytes: LSB first, then MSB
    *als_counts = (data[1] << 8) | data[0];
    
    // Convert to lux if pointer is provided
    if (lux != NULL) {
        *lux = (*als_counts) * veml7700_resolution;
    }
    return HAL_OK;
}

HAL_StatusTypeDef VEML7700_ReadWHITE(I2C_HandleTypeDef *hi2c, uint16_t *white_counts) {
    HAL_StatusTypeDef status;
    uint8_t data[2];
    
    // Read 2 bytes from WHITE register (simpler!)
    status = HAL_I2C_Mem_Read(hi2c, (VEML7700_I2C_ADDR << 1), VEML7700_REG_WHITE_DATA, I2C_MEMADD_SIZE_8BIT, data, 2, HAL_MAX_DELAY);
    
    if (status != HAL_OK) {
        return status;
    }
    
    // Combine bytes: LSB first, then MSB
    *white_counts = (data[1] << 8) | data[0];
    
    return HAL_OK;
}