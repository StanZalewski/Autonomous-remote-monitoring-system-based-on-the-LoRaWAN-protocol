#include "init.h"
#include "main.h"
#include "bq25798Charger.h"
#include "bq76952BMS.h"
#include "stm32g0xx_hal.h"
#include "control.h"

FaultCheckflags g_fault_check = {0};
ChargerStatusflags g_charger_status = {0};
BMSStatusFlags g_bms_status = {0};

HAL_StatusTypeDef BQ25798_HealthCheck(void)
{
    HAL_StatusTypeDef status;
    
    status = BQ25798_DeviceCheck(&hi2c1);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }
    
    status = BQ25798_FaultCheck(&hi2c1, &g_fault_check.fault_check_0, &g_fault_check.fault_check_1);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }
    
    status = BQ25798_FaultFlags(&hi2c1, &g_fault_check.fault_flag_0, &g_fault_check.fault_flag_1);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }

    status = BQ25798_ChargerFlags(&hi2c1, &g_charger_status.charger_flag_0, &g_charger_status.charger_flag_1, &g_charger_status.charger_flag_2, &g_charger_status.charger_flag_3);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }
    

    status = BQ25798_ChargerStatus(&hi2c1, &g_charger_status.charger_status_0, &g_charger_status.charger_status_1, &g_charger_status.charger_status_2, &g_charger_status.charger_status_3, &g_charger_status.charger_status_4);
    if (status != HAL_OK) {
        return HAL_ERROR;
    }

    if (g_fault_check.fault_check_0 != 0 || g_fault_check.fault_check_1 != 0 ||
        g_fault_check.fault_flag_0 != 0 || g_fault_check.fault_flag_1 != 0) {
        return HAL_ERROR;
    }
    return HAL_OK;
}

HAL_StatusTypeDef BQ76952_HealthCheck(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;
    
    // Device Check
    status = BQ76952_ReadRegister(hi2c, 0x12, &g_bms_status.battery_status, 1);
    if (status != HAL_OK) return HAL_ERROR;
    
    // Safety Alert A, B, C
    status = BQ76952_ReadRegister(hi2c, 0x02, &g_bms_status.safety_alert_a, 1);
    if (status != HAL_OK) return HAL_ERROR;
    status = BQ76952_ReadRegister(hi2c, 0x04, &g_bms_status.safety_alert_b, 1);
    if (status != HAL_OK) return HAL_ERROR;
    status = BQ76952_ReadRegister(hi2c, 0x06, &g_bms_status.safety_alert_c, 1);
    if (status != HAL_OK) return HAL_ERROR;
    
    // Safety Status A, B, C
    status = BQ76952_ReadRegister(hi2c, 0x03, &g_bms_status.safety_status_a, 1);
    if (status != HAL_OK) return HAL_ERROR;
    status = BQ76952_ReadRegister(hi2c, 0x05, &g_bms_status.safety_status_b, 1);
    if (status != HAL_OK) return HAL_ERROR;
    status = BQ76952_ReadRegister(hi2c, 0x07, &g_bms_status.safety_status_c, 1);
    if (status != HAL_OK) return HAL_ERROR;
    
    // PF Alert A, B, C, D
    status = BQ76952_ReadRegister(hi2c, 0x0A, &g_bms_status.pf_alert_a, 1);
    if (status != HAL_OK) return HAL_ERROR;
    status = BQ76952_ReadRegister(hi2c, 0x0C, &g_bms_status.pf_alert_b, 1);
    if (status != HAL_OK) return HAL_ERROR;
    status = BQ76952_ReadRegister(hi2c, 0x0E, &g_bms_status.pf_alert_c, 1);
    if (status != HAL_OK) return HAL_ERROR;
    status = BQ76952_ReadRegister(hi2c, 0x10, &g_bms_status.pf_alert_d, 1);
    if (status != HAL_OK) return HAL_ERROR;
    
    // PF Status A, B, C, D
    status = BQ76952_ReadRegister(hi2c, 0x0B, &g_bms_status.pf_status_a, 1);
    if (status != HAL_OK) return HAL_ERROR;
    status = BQ76952_ReadRegister(hi2c, 0x0D, &g_bms_status.pf_status_b, 1);
    if (status != HAL_OK) return HAL_ERROR;
    status = BQ76952_ReadRegister(hi2c, 0x0F, &g_bms_status.pf_status_c, 1);
    if (status != HAL_OK) return HAL_ERROR;
    status = BQ76952_ReadRegister(hi2c, 0x11, &g_bms_status.pf_status_d, 1);
    if (status != HAL_OK) return HAL_ERROR;
    
    // Check for permanent fails
    if (g_bms_status.pf_status_a != 0 || g_bms_status.pf_status_b != 0 ||
        g_bms_status.pf_status_c != 0 || g_bms_status.pf_status_d != 0) {
        return HAL_ERROR;
    }
    
    return HAL_OK;
}



void BQ25798_Init()
{
    BQ25798_SetChargerControl_0(&hi2c1, DISABLED, ENABLED, DISABLED, DO_NOT_FORCE_ICO, DISABLED, DISABLED, IDLE, ENABLED);
    HAL_Delay(50);
    
    BQ25798_SetChargerControl_5(&hi2c1, DISABLED, DISABLED, ENABLED, 
                               IDSG_REGULATION_DISABLED, DISABLED, NO_SHIP_FET);
    
    BQ25798_SetRechargeControl(&hi2c1, 150, RECHARGE_TIME_1024_MS, CELL_COUNT_3);
    HAL_Delay(1000);
    BQ25798_SetMinSystemVoltage(&hi2c1, 8500);
    BQ25798_SetChargeVoltageLimit(&hi2c1, 10800);
    BQ25798_SetChargeCurrentLimit(&hi2c1, 1300);
    BQ25798_SetInputCurrentLimit(&hi2c1, 1670);
    
    BQ25798_SetPrechargeParam(&hi2c1, 160, VBAT_LOWV_71_4_PERCENT);
    BQ25798_SetTerminationControl(&hi2c1, 40, ENABLED, NO_RESET);
    BQ25798_SetTimerControl(&hi2c1, ENABLED, TWELVE_HRS, ENABLED, ENABLED, ENABLED, TOP_OFF_TIMER_DISABLE);
    
    BQ25798_SetChargerControl_1(&hi2c1, TIMER_80s, WD_RESET, VAC_OVP_26V, VINDPM_60P);
    BQ25798_SetChargerControl_2(&hi2c1, ADD_10S_DELAY, DEVICE_IDLE, DISABLED, DISABLED, DISABLED, 
                               DISABLED, DO_NOT_FORCE_D_TERMINALS_DETECTION);
    BQ25798_SetChargerControl_3(&hi2c1, DISABLE_OOA_IN_FORWARD_ENABLED, DISABLE_OOA_IN_OTG_ENABLED, 
                               DISABLE_BATFET_LDO_IN_PCHG_ENABLED, WAKE_UP_FROM_SHIPMODE_1S, 
                               PFM_IN_FORWARD_ENABLED, PFM_IN_OTG_ENABLE, DISABLED, ACDRV_1_2_UP);
    BQ25798_SetChargerControl_4(&hi2c1, IBUS_OCP_IN_FORWARD_ENABLE, DO_NOT_FORCE_VINDPM_DETECTION, 
                               VOTG_UVP_HICCUP_PROTECTION_ENABLE, HICCUP_PROTECTION_ENABLE, 
                               STAT_PIN_ENABLE, FREQ_750_KHZ, TURN_ON_ACDRV1, TURN_OFF_ACDRV2);
    BQ25798_SetADCControl(&hi2c1, ENABLED, ADC_CONTINUOUS, ADC_15_BIT, ADC_RUNNING_AVERAGE, ADC_AVG_USE_EXISTING);
    
    BQ25798_SetTemperatureControl(&hi2c1, ACFET1_IDLE, ENABLED, DISABLED, DISABLED, 
                                 TEMP_SHUTDOWN_150_C, TEMP_100_C);
    BQ25798_SetNTCCONTROL_0(&hi2c1, SET_ICHG_TO_20_PERC_OF_ICHG, SET_ICHG_TO_40_PERC_OF_ICHG, 
                           SET_VREG_TO_VREG_400mV);
    BQ25798_SetNTCCONTROL_1(&hi2c1, ENABLED,
                        OTG_TS_COLD_MIN_10_C,
                        OTG_TS_HOT_65_C,
                        PERCENT_37_7_AT_55_C,
                        PERCENT_71_1_AT_5_C);


    
    }


HAL_StatusTypeDef BQ76952_Init(void)
{
    HAL_StatusTypeDef status;
    
    // Enter CONFIG_UPDATE mode
    status = BQ76952_EnterConfigUpdateMode(&hi2c1);
    if (status != HAL_OK) return status;
    HAL_Delay(10);
    
    // Configure basic settings (power, regulators, vcell mode)
    status = BQ76952_InitBasicConfig(&hi2c1);
    if (status != HAL_OK) return status;
    
    // Configure protection enable/FET action bits
    status = BQ76952_InitProtectionSettings(&hi2c1);
    if (status != HAL_OK) return status;
    
    // Configure protection thresholds and delays
    status = BQ76952_InitProtectionValues(&hi2c1);
    if (status != HAL_OK) return status;
    
    // Configure FETs and pin assignments
    status = BQ76952_InitFETAndPins(&hi2c1);
    if (status != HAL_OK) return status;

    status = BQ76952_InitCellBalancing(&hi2c1);
    
    // Exit CONFIG_UPDATE mode and apply settings
    status = BQ76952_ExitConfigUpdate(&hi2c1);
    if (status != HAL_OK) return status;
    
    HAL_Delay(100);
    
    return HAL_OK;
}

HAL_StatusTypeDef fullInitialization(void)
{
    HAL_StatusTypeDef status;


    // Check BMS
    status = BQ76952_HealthCheck(&hi2c1);
    if (status != HAL_OK) {
        return HAL_ERROR;  // BMS not responding or has permanent fail
    }

    BQ25798_Init();

    status = BQ76952_Init();
    if (status != HAL_OK) {
        return HAL_ERROR;  
    }
    
    status = BQ25798_VerifyConfiguration(&hi2c1);
    if (status != HAL_OK) {
        return HAL_ERROR;  
    }
    
    status = BQ76952_VerifyCritical(&hi2c1);
    if (status != HAL_OK) {
        return HAL_ERROR;  
    }

    GenSysInfo();

    if (g_bms_status.pf_status_a == 0 && g_bms_status.pf_status_b == 0) {
        uint8_t data[2];
        data[0] = 0x96;  // LSB
        data[1] = 0x00;  // MSB
        status = BQ76952_WriteRegister(&hi2c1, 0x3E, data, 2);
        if (status != HAL_OK) {
            return HAL_ERROR;  // Command failed!
        }
    }
    // 3. Wait for FETs to turn on
    HAL_Delay(100);
    status = GenSysInfo();
    if (status != HAL_OK) {
        return HAL_ERROR;  
    }
    //Opening FETS

    HAL_GPIO_WritePin(CE_GPIO_Port, CE_Pin, GPIO_PIN_SET);  // CE HIGH = disabled
    HAL_Delay(1000);

    // Re-enable
    HAL_GPIO_WritePin(CE_GPIO_Port, CE_Pin, GPIO_PIN_RESET);  // CE LOW = enabled
    HAL_Delay(1000);

    BQ25798_SetChargerControl_0(&hi2c1, DISABLED, ENABLED, DISABLED, DO_NOT_FORCE_ICO, DISABLED, ENABLED, IDLE, ENABLED);
    HAL_Delay(500);

    //BQ25798_SetMPPTControl(&hi2c1, ENABLE_MPPT, T_DELAY_AFTER_SWITCHING_2MIN, T_DELAY_AFTER_SWITCHING_300MS, VINDPM_PERCANTAGE_OF_VOC_0_8125);
    
    
    return HAL_OK;
}