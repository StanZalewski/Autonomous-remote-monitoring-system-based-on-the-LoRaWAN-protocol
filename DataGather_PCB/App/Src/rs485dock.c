#include "rs485dock.h"
#include "modbus_rtu.h"
#include "datacollect.h"

// External sensor data (defined in datacollect.c)
extern float temperature_sht45;
extern float humidity_sht45;
extern float lux_veml7700;
extern float whiteRatio_veml7700;
extern float temperature_bmp390;
extern float pressure_bmp390;

// Register addresses - OPTIMIZED (8 registers total, down from 12)
#define REG_TEMP_SHT45      0   // uint16 - Temperature ×100
#define REG_HUMIDITY_SHT45  1   // uint16 - Humidity ×100
#define REG_LUX_VEML7700    2   // uint32 - Lux (direct) - uses 2-3
#define REG_WHITE_VEML7700  4   // uint16 - White ratio ×1000
#define REG_TEMP_BMP390     5   // uint16 - Temperature ×100
#define REG_PRESSURE_BMP390 6   // uint32 - Pressure Pa (direct) - uses 6-7

// Scaling factors
#define SCALE_TEMPERATURE   100.0f   // °C → 25.78 becomes 2578
#define SCALE_HUMIDITY      100.0f   // %  → 45.23 becomes 4523
#define SCALE_WHITE_RATIO   1000.0f  // ratio → 0.876 becomes 876

void RS485_Init(UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim)
{
    Modbus_Init(huart, htim);
}

void RS485_UpdateRegisters(void)
{
    // Temperature SHT45 - uint16, scaled ×100
    // Range: -40 to +125°C → -4000 to +12500 (fits in int16)
    int16_t temp_sht45_scaled = (int16_t)(temperature_sht45 * SCALE_TEMPERATURE);
    Modbus_SetRegisterU16(REG_TEMP_SHT45, (uint16_t)temp_sht45_scaled);
    
    // Humidity SHT45 - uint16, scaled ×100
    // Range: 0 to 100% → 0 to 10000
    uint16_t humidity_scaled = (uint16_t)(humidity_sht45 * SCALE_HUMIDITY);
    Modbus_SetRegisterU16(REG_HUMIDITY_SHT45, humidity_scaled);
    
    // Lux VEML7700 - uint32, direct value
    // Can exceed 65535, needs 2 registers
    uint32_t lux_value = (uint32_t)lux_veml7700;
    Modbus_SetRegisterU32(REG_LUX_VEML7700, lux_value);
    
    // White ratio - uint16, scaled ×1000
    // Range: 0 to ~65 (0.000 to 65.535)
    uint16_t white_ratio_scaled = (uint16_t)(whiteRatio_veml7700 * SCALE_WHITE_RATIO);
    Modbus_SetRegisterU16(REG_WHITE_VEML7700, white_ratio_scaled);
    
    // Temperature BMP390 - uint16, scaled ×100
    // Range: -40 to +85°C → -4000 to +8500 (fits in int16)
    int16_t temp_bmp390_scaled = (int16_t)(temperature_bmp390 * SCALE_TEMPERATURE);
    Modbus_SetRegisterU16(REG_TEMP_BMP390, (uint16_t)temp_bmp390_scaled);
    
    // Pressure BMP390 - uint32, direct Pa
    // Range: ~30000 to 110000 Pa (exceeds uint16 max of 65535)
    uint32_t pressure_value = (uint32_t)pressure_bmp390;
    Modbus_SetRegisterU32(REG_PRESSURE_BMP390, pressure_value);
}

void RS485_Process(void)
{

    Modbus_Process();  // Your existing call
}

