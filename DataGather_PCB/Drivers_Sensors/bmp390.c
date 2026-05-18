#include "bmp390.h"
#include "stm32g0xx_hal_i2c.h"


HAL_StatusTypeDef BMP390_ERROR_REGISTER(I2C_HandleTypeDef *hi2c, uint8_t *error_reg){

    return HAL_I2C_Mem_Read(hi2c, (BMP390_I2C_ADDR_SDO_GND << 1), BMP390_REG_ERROR, I2C_MEMADD_SIZE_8BIT, error_reg, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef BMP390_CHIP_ID(I2C_HandleTypeDef *hi2c, uint8_t *chip_id){

    return HAL_I2C_Mem_Read(hi2c, (BMP390_I2C_ADDR_SDO_GND << 1), BMP390_REG_CHIP_ID, I2C_MEMADD_SIZE_8BIT, chip_id, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef BMP390_STATUS(I2C_HandleTypeDef *hi2c, uint8_t *status){

    return HAL_I2C_Mem_Read(hi2c, (BMP390_I2C_ADDR_SDO_GND << 1), BMP390_REG_STATUS, I2C_MEMADD_SIZE_8BIT, status, 1, HAL_MAX_DELAY);
}

/**
 * @brief Read calibration data and convert NVM values to floating-point parameters
 * CRITICAL: The NVM values must be converted using scaling factors from the datasheet!
 */
HAL_StatusTypeDef BMP390_ReadCalibration(I2C_HandleTypeDef *hi2c, BMP390_CalibData *calib) {
    uint8_t calib_data[21];  // 21 bytes of calibration data
    HAL_StatusTypeDef status;
    
    // Read 21 bytes starting from address 0x31
    status = HAL_I2C_Mem_Read(hi2c, (BMP390_I2C_ADDR_SDO_GND << 1), BMP390_REG_CALIB_DATA, I2C_MEMADD_SIZE_8BIT, calib_data, 21, HAL_MAX_DELAY);
    
    if (status != HAL_OK) {
        return status;
    }
    
    // Parse NVM calibration data according to datasheet Table 24
    // Temperature coefficients (NVM values - will be converted below)
    uint16_t nvm_par_t1 = (uint16_t)(calib_data[1] << 8) | calib_data[0];   // 0x31-0x32
    uint16_t nvm_par_t2 = (uint16_t)(calib_data[3] << 8) | calib_data[2];   // 0x33-0x34
    int8_t nvm_par_t3 = (int8_t)calib_data[4];                              // 0x35
    
    // Pressure coefficients (NVM values - will be converted below)
    int16_t nvm_par_p1  = (int16_t)((calib_data[6] << 8) | calib_data[5]);   // 0x36-0x37
    int16_t nvm_par_p2  = (int16_t)((calib_data[8] << 8) | calib_data[7]);   // 0x38-0x39
    int8_t nvm_par_p3  = (int8_t)calib_data[9];                              // 0x3A
    int8_t nvm_par_p4  = (int8_t)calib_data[10];                             // 0x3B
    uint16_t nvm_par_p5  = (uint16_t)(calib_data[12] << 8) | calib_data[11]; // 0x3C-0x3D
    uint16_t nvm_par_p6  = (uint16_t)(calib_data[14] << 8) | calib_data[13]; // 0x3E-0x3F
    int8_t nvm_par_p7  = (int8_t)calib_data[15];                             // 0x40
    int8_t nvm_par_p8  = (int8_t)calib_data[16];                             // 0x41
    int16_t nvm_par_p9  = (int16_t)((calib_data[18] << 8) | calib_data[17]); // 0x42-0x43
    int8_t nvm_par_p10 = (int8_t)calib_data[19];                             // 0x44
    int8_t nvm_par_p11 = (int8_t)calib_data[20];                             // 0x45
    
    // Convert NVM values to floating-point compensation parameters
    // These formulas are from the datasheet Appendix (page 55)
    
    // Temperature parameter conversions
    calib->par_t1 = ((float)nvm_par_t1 / 0.00390625f);         // 2^-8 = 0.00390625, so divide by it = multiply by 256
    calib->par_t2 = ((float)nvm_par_t2 / 1073741824.0f);       // 2^30
    calib->par_t3 = ((float)nvm_par_t3 / 281474976710656.0f);  // 2^48
    
    // Pressure parameter conversions
    calib->par_p1 = ((float)(nvm_par_p1 - 16384) / 1048576.0f);        // (NVM - 2^14) / 2^20
    calib->par_p2 = ((float)(nvm_par_p2 - 16384) / 536870912.0f);      // (NVM - 2^14) / 2^29
    calib->par_p3 = ((float)nvm_par_p3 / 4294967296.0f);               // 2^32
    calib->par_p4 = ((float)nvm_par_p4 / 137438953472.0f);             // 2^37
    calib->par_p5 = ((float)nvm_par_p5 / 0.125f);                      // 2^-3 = 0.125, so divide by it = multiply by 8
    calib->par_p6 = ((float)nvm_par_p6 / 64.0f);                       // 2^6
    calib->par_p7 = ((float)nvm_par_p7 / 256.0f);                      // 2^8
    calib->par_p8 = ((float)nvm_par_p8 / 32768.0f);                    // 2^15
    calib->par_p9 = ((float)nvm_par_p9 / 281474976710656.0f);          // 2^48
    calib->par_p10 = ((float)nvm_par_p10 / 281474976710656.0f);        // 2^48
    calib->par_p11 = ((float)nvm_par_p11 / 36893488147419103232.0f);   // 2^65
    
    calib->t_lin = 0.0f;  // Initialize
    
    return HAL_OK;
}

HAL_StatusTypeDef BMP390_ReadRawTemperature(I2C_HandleTypeDef *hi2c, uint32_t *raw_temp) {
    uint8_t data[3];
    HAL_StatusTypeDef status;
    
    status = HAL_I2C_Mem_Read(hi2c, (BMP390_I2C_ADDR_SDO_GND << 1), BMP390_REG_DATA_3, I2C_MEMADD_SIZE_8BIT, data, 3, HAL_MAX_DELAY);
    
    if (status != HAL_OK) {
        return status;
    }
    
    *raw_temp = ((uint32_t)data[2] << 16) | ((uint32_t)data[1] << 8) | data[0];
    
    return HAL_OK;
}

/**
 * @brief Compensate temperature using converted calibration parameters
 * This matches the datasheet reference code exactly (page 55)
 */
float BMP390_CompensateTemperature(uint32_t raw_temp, BMP390_CalibData *calib) {
    float partial_data1;
    float partial_data2;

    // Reference formula from datasheet (uses converted parameters)
    partial_data1 = (float)(raw_temp - calib->par_t1);
    partial_data2 = partial_data1 * calib->par_t2;

    // Store t_lin in calibration structure for pressure compensation
    calib->t_lin = partial_data2 + (partial_data1 * partial_data1) * calib->par_t3;

    return calib->t_lin;
}

HAL_StatusTypeDef BMP390_ReadRawPressure(I2C_HandleTypeDef *hi2c, uint32_t *raw_press) {
    uint8_t data[3];
    HAL_StatusTypeDef status;
    

    status = HAL_I2C_Mem_Read(hi2c, (BMP390_I2C_ADDR_SDO_GND << 1), BMP390_REG_DATA_0, I2C_MEMADD_SIZE_8BIT, data, 3, HAL_MAX_DELAY);
    
    if (status != HAL_OK) {
        return status;
    }

    *raw_press = ((uint32_t)data[2] << 16) | ((uint32_t)data[1] << 8) | data[0];
    
    return HAL_OK;
}

/**
 * @brief Compensate pressure using converted calibration parameters
 * This matches the datasheet reference code exactly (page 56)
 */
float BMP390_CompensatePressure(uint32_t raw_press, BMP390_CalibData *calib) {
    float partial_data1;
    float partial_data2;
    float partial_data3;
    float partial_data4;
    float partial_out1;
    float partial_out2;
    float t_lin = calib->t_lin;  // Use stored t_lin from temperature compensation

    // Reference formula from datasheet (uses converted parameters)
    partial_data1 = calib->par_p6 * t_lin;
    partial_data2 = calib->par_p7 * (t_lin * t_lin);
    partial_data3 = calib->par_p8 * (t_lin * t_lin * t_lin);
    partial_out1 = calib->par_p5 + partial_data1 + partial_data2 + partial_data3;
    
    partial_data1 = calib->par_p2 * t_lin;
    partial_data2 = calib->par_p3 * (t_lin * t_lin);
    partial_data3 = calib->par_p4 * (t_lin * t_lin * t_lin);
    partial_out2 = (float)raw_press * (calib->par_p1 + partial_data1 + partial_data2 + partial_data3);
    
    partial_data1 = (float)raw_press * (float)raw_press;
    partial_data2 = calib->par_p9 + calib->par_p10 * t_lin;
    partial_data3 = partial_data1 * partial_data2;
    partial_data4 = partial_data3 + ((float)raw_press * (float)raw_press * (float)raw_press) * calib->par_p11;
    
    return partial_out1 + partial_out2 + partial_data4;
}

/**
 * @brief Wait for data to be ready
 */
HAL_StatusTypeDef BMP390_WaitForData(I2C_HandleTypeDef *hi2c, uint32_t timeout_ms) {
    uint8_t status;
    uint32_t start = HAL_GetTick();
    
    do {
        if (BMP390_STATUS(hi2c, &status) != HAL_OK) {
            return HAL_ERROR;
        }
        
        // Check if both temperature and pressure data are ready
        if ((status & BMP390_STATUS_DRDY_PRESS) && (status & BMP390_STATUS_DRDY_TEMP)) {
            return HAL_OK;
        }
        
        if ((HAL_GetTick() - start) > timeout_ms) {
            return HAL_TIMEOUT;
        }
        
        HAL_Delay(1);
    } while(1);
}

/**
 * @brief Initialize BMP390 sensor with proper configuration
 */
HAL_StatusTypeDef BMP390_Init(I2C_HandleTypeDef *hi2c) {
    HAL_StatusTypeDef status;
    uint8_t cmd;
    uint8_t chip_id;
    
    // 1. Verify chip ID
    status = BMP390_CHIP_ID(hi2c, &chip_id);
    if (status != HAL_OK || chip_id != 0x60) {
        return HAL_ERROR;  // Wrong chip ID
    }
    
    // 2. Soft reset
    cmd = 0xB6;  // Soft reset command
    status = HAL_I2C_Mem_Write(hi2c, (BMP390_I2C_ADDR_SDO_GND << 1), BMP390_REG_CMD,
                               I2C_MEMADD_SIZE_8BIT, &cmd, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return status;
    }
    
    // Wait for reset to complete
    HAL_Delay(10);
    
    // 3. Configure oversampling (OSR)
    // osr_p = x8 (011b), osr_t = x1 (000b)
    uint8_t osr = 0x03;  // bits [2:0] = 011 (x8 pressure), bits [5:3] = 000 (x1 temp)
    status = HAL_I2C_Mem_Write(hi2c, (BMP390_I2C_ADDR_SDO_GND << 1), BMP390_REG_OSR,
                               I2C_MEMADD_SIZE_8BIT, &osr, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return status;
    }
    
    // 4. Configure output data rate (ODR)
    // ODR = 50Hz (0x02)
    uint8_t odr = 0x02;
    status = HAL_I2C_Mem_Write(hi2c, (BMP390_I2C_ADDR_SDO_GND << 1), BMP390_REG_ODR,
                               I2C_MEMADD_SIZE_8BIT, &odr, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return status;
    }
    
    // 5. Configure IIR filter
    // Filter coefficient = 3 (010b in bits [3:1])
    uint8_t config = 0x04;  // 010 << 1 = 0x04
    status = HAL_I2C_Mem_Write(hi2c, (BMP390_I2C_ADDR_SDO_GND << 1), BMP390_REG_CONFIG,
                               I2C_MEMADD_SIZE_8BIT, &config, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return status;
    }
    
    // 6. Enable sensor in normal mode
    // mode = 11b (normal), temp_en = 1, press_en = 1
    // PWR_CTRL register: bits [5:4] = mode, bit [1] = temp_en, bit [0] = press_en
    uint8_t pwr_ctrl = 0x33;  // 0b00110011 = normal mode + both sensors enabled
    status = HAL_I2C_Mem_Write(hi2c, (BMP390_I2C_ADDR_SDO_GND << 1), BMP390_REG_PWR_CTRL,
                               I2C_MEMADD_SIZE_8BIT, &pwr_ctrl, 1, HAL_MAX_DELAY);
    if (status != HAL_OK) {
        return status;
    }
    
    // Wait for first measurement to be ready
    HAL_Delay(50);
    
    return HAL_OK;
}