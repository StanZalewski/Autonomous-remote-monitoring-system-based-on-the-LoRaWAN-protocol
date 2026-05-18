#include "control.h"
#include "main.h" 
#include "bq25798Charger.h"
#include "bq76952BMS.h"
#include "init.h" 
#include "stm32g0xx_hal_def.h"


SystemData g_system_data = {0};


HAL_StatusTypeDef GenSysInfo(void)
{
    HAL_StatusTypeDef status;
    uint8_t data[2];
    int16_t temp_kelvin;
    uint8_t temp_reg;
    
    // ========== 1. Read Cell Voltages ==========
    
    // Cell 1 Voltage (0x14)
    status = BQ76952_ReadRegister(&hi2c1, 0x14, data, 2);
    if (status != HAL_OK) return status;
    g_system_data.cell1_voltage = (int16_t)(data[0] | (data[1] << 8));
    
    // Cell 2 Voltage (0x16)
    status = BQ76952_ReadRegister(&hi2c1, 0x16, data, 2);
    if (status != HAL_OK) return status;
    g_system_data.cell2_voltage = (int16_t)(data[0] | (data[1] << 8));
    
    // Cell 3 Voltage (0x32 - Cell 16 register)
    status = BQ76952_ReadRegister(&hi2c1, 0x32, data, 2);
    if (status != HAL_OK) return status;
    g_system_data.cell3_voltage = (int16_t)(data[0] | (data[1] << 8));
    
    // ========== 2. Read Current ==========
    
    // Read CC2 Current (0x3A) - Signed, in mA
    status = BQ76952_ReadRegister(&hi2c1, 0x3A, data, 2);
    if (status != HAL_OK) return status;
    g_system_data.current_pack = (int16_t)(data[0] | (data[1] << 8));
    
    // ========== 3. Read Pack Temperature BMS Side ==========
    
    // Read TS1 Temperature (0x70) - Returns 0.1K
    status = BQ76952_ReadRegister(&hi2c1, 0x70, data, 2);
    if (status != HAL_OK) return status;
    temp_kelvin = (int16_t)(data[0] | (data[1] << 8));
    
    // Convert from 0.1K to 0.1°C (subtract 273.1 * 10 = 2731)
    g_system_data.pack_temperature_BMS = temp_kelvin - 2731;

    // ========== 4. Read Charger Temperature ==========
    
    // Read TS_ADC (0x3F - BQ25798) - Returns percentage (0-99.9023%)
    status = BQ25798_ReadRegister(&hi2c1, 0x3F, data, 2);
    if (status != HAL_OK) return status;
    uint16_t ts_adc_raw = (uint16_t)((data[0] << 8) | data[1]);
    
    // Convert to percentage: percentage = raw_value × 0.0976563%
    // For storage as integer (0.1% resolution): percentage_tenth = raw_value * 0.976563
    // Or just store raw value for now
    g_system_data.pack_temperature_Charger = (int16_t)ts_adc_raw;
    
    // ========== 5. Read VAC1 Voltage (BQ25798 - BIG ENDIAN!) ==========
    
    // Read VAC1_ADC (0x37 - BQ25798) - MSB FIRST!
    status = BQ25798_ReadRegister(&hi2c1, 0x37, data, 2);
    if (status != HAL_OK) return status;
    g_system_data.vac1_voltage = (uint16_t)((data[0] << 8) | data[1]);
    
    
    // ========== 6. Build gen_sys_status Byte ==========
    
    g_system_data.gen_sys_status = 0;
    
    // Bit 0: CHG FET Status (DCHG pin on PA5, active-LOW polarity)
    // Pin HIGH = CHG FET ON
    if (HAL_GPIO_ReadPin(DCHG_GPIO_Port, DCHG_Pin) == GPIO_PIN_SET) {
        g_system_data.gen_sys_status |= (1 << 0);
    }
    
    // Bit 1: DSG FET Status (DDSG pin on PA4, active-LOW polarity)
    // Pin HIGH = DSG FET ON
    if (HAL_GPIO_ReadPin(DDSG_GPIO_Port, DDSG_Pin) == GPIO_PIN_SET) {
        g_system_data.gen_sys_status |= (1 << 1);
    }
    
    // Bit 2: MPPT Status (from MPPT_Control register 0x15, bit 0 = EN_MPPT)
    status = BQ25798_ReadRegister(&hi2c1, 0x15, &temp_reg, 1);
    if (status != HAL_OK) return status;
    if (temp_reg & 0x01) {  // Bit 0 = EN_MPPT
        g_system_data.gen_sys_status |= (1 << 2);
    }
    
    // Bit 3: Battery Presence Status (from charger_status_2, bit 0)
    // Copy bit 0 from g_charger_status.charger_status_2 to bit 3
    if (g_charger_status.charger_status_2 & 0x01) {
        g_system_data.gen_sys_status |= (1 << 3);
    }
    
    // Bit 4: Watchdog Status (from charger_status_0, bit 5)
    // Bit 5 of charger_status_0 is typically WD_STAT or similar
    // Copy bit 5 to bit 4 of gen_sys_status
    if (g_charger_status.charger_status_0 & (1 << 5)) {
        g_system_data.gen_sys_status |= (1 << 4);
    }
    
    // Bits 5-7: Charger Status (CHG_STAT from charger_status_1 bits 7:5)
    // Extract bits 7:5 and shift to bits 7:5 of gen_sys_status
    uint8_t chg_stat = (g_charger_status.charger_status_1 >> 5) & 0x07;  // Get bits 7:5
    g_system_data.gen_sys_status |= (chg_stat << 5);  // Place in bits 7:5
    
    return HAL_OK;
}



void WatchDogs_R (void)
{
    BQ25798_SetChargerControl_1(&hi2c1, TIMER_80s, WD_RESET, VAC_OVP_26V, VINDPM_60P);
}