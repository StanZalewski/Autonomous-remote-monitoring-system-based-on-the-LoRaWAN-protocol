#include "sht45.h"
#include "stm32g0xx_hal_i2c.h"

#define SHT45_I2C_ADDR   (0x44 << 1)
/* High precision measurement command (no heater) */
#define SHT45_CMD_MEASURE_MEDIUMPRECISION  0xF6

HAL_StatusTypeDef SHT45_ReadTempRH(I2C_HandleTypeDef *hi2c, float *temperature_degC, float *humidity_pRH){

    HAL_StatusTypeDef status;
    uint8_t cmd = SHT45_CMD_MEASURE_MEDIUMPRECISION;
    uint8_t rx_buf[6];

    uint16_t t_ticks;
    uint16_t rh_ticks;

    /* Send measurement command */
    status = HAL_I2C_Master_Transmit(hi2c, SHT45_I2C_ADDR, &cmd, 1,HAL_MAX_DELAY);
    if (status != HAL_OK)
        return status;

    HAL_Delay(10);

    /* Read measurement data */
    status = HAL_I2C_Master_Receive(hi2c, SHT45_I2C_ADDR, rx_buf, 6, HAL_MAX_DELAY);
    if (status != HAL_OK)
        return status;

    /* Parse raw values */
    t_ticks  = (uint16_t)(rx_buf[0] << 8) | rx_buf[1];
    rh_ticks = (uint16_t)(rx_buf[3] << 8) | rx_buf[4];

    /* Convert to physical values (datasheet formulas) */
    *temperature_degC = -45.0f + 175.0f * ((float)t_ticks / 65535.0f);
    *humidity_pRH     = -6.0f  + 125.0f * ((float)rh_ticks / 65535.0f);

    /* Clamp humidity to [0, 100] */
    if (*humidity_pRH > 100.0f)
        *humidity_pRH = 100.0f;
    else if (*humidity_pRH < 0.0f)
        *humidity_pRH = 0.0f;

    return HAL_OK;
}



