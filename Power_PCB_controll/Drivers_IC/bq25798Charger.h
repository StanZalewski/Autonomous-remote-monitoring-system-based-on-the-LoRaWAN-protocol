#ifndef BQ25798_CHARGER_H
#define BQ25798_CHARGER_H

#include "stm32g0xx_hal.h"
#include <stdint.h>


#define BQ25798_I2C_ADDRESS     (0x6B << 1)
#define BQ25798_I2C_TIMEOUT     100
#define BQ25798_REG_MAX         0x48



//Multipurpose enum
typedef enum{
    DISABLED = 0,
    ENABLED  = 1,
}select;

// Battery transition treshold from slow to fast charging (percentage of VREG)
typedef enum {
    VBAT_LOWV_15_PERCENT   = 0,   
    VBAT_LOWV_62_2_PERCENT = 1,
    VBAT_LOWV_66_7_PERCENT = 2,
    VBAT_LOWV_71_4_PERCENT = 3
} vbat_lowv_t;

// State of reseting registers to default values 
typedef enum {
    NO_RESET        = 0,   
    RESET_REGISTERS = 1
} reset_state;

// Number of cells in series connected to the charger
typedef enum{
    CELL_COUNT_1 = 0,
    CELL_COUNT_2 = 1,
    CELL_COUNT_3 = 2,
    CELL_COUNT_4 = 3,
} cell_count;

//Battery recharge deglich time
typedef enum{
    RECHARGE_TIME_64_MS   = 0,
    RECHARGE_TIME_256_MS  = 1,
    RECHARGE_TIME_1024_MS = 2,
    RECHARGE_TIME_2048_MS = 3,
} rechg_deglich_time;

//Top-off timer control
typedef enum{
    TOP_OFF_TIMER_DISABLE = 0,
    TOP_OFF_TIMER_15_MIN   = 1,
    TOP_OFF_TIMER_30_MIN   = 2,
    TOP_OFF_TIMER_45_MIN   = 3,
} top_off_tmr_ctrl;

//Fast charge timer setting
typedef enum{
    FIVE_HRS        = 0,
    EIGHT_HRS       = 1,
    TWELVE_HRS      = 2,
    TWENTY_FOUR_HRS = 3,
}fst_chg_tmr_ctrl;

//Force the charger to apply a discharging current on BAT regardless the battery OVP status
typedef enum{
    IDLE                                = 0,
    FORCE_DCHG_CURRENT_OVP_FAULT_ENABLE = 1,
}force_ibatdis;

//Force start input current optimizer (ICO)
typedef enum{
    DO_NOT_FORCE_ICO   = 0,
    FORCE_ICO_AT_START = 1,
}force_ico;

//Thresholds to trigger the backup mode
typedef enum{
    VINDPM_40P  = 0,
    VINDPM_60P  = 1,
    VINDPM_80P  = 2,
    VINDPM_100P = 3,
}vbus_bckp;

//VAC over voltage protection thresholds
typedef enum{
    VAC_OVP_26V = 0,
    VAC_OVP_22V = 1,
    VAC_OVP_12V = 2,
    VAC_OVP_7V  = 3,
}vac_ovp_t;

//I2C watch dog timer reset
typedef enum{
    WD_NORMAL = 0,
    WD_RESET = 1,
}wd_reset;

//Watchdog timer settings
typedef enum{
    TIMER_DISABLE = 0,
    TIMER_0_5s    = 1,
    TIMER_1s      = 2,
    TIMER_2s      = 3,
    TIMER_20s     = 4,
    TIMER_40s     = 5,
    TIMER_80s     = 6,
    TIMER_160s    = 7,
}wd;

//Force D+/D- detection
typedef enum{
    DO_NOT_FORCE_D_TERMINALS_DETECTION = 0,
    FORCE_D_TERMINALS_DETECTION        = 1,
}f_indet;

//The external ship FET control logic to force the device enter different modes.
typedef enum{
    DEVICE_IDLE        = 0,
    DEVICE_SHUTDOWN    = 1,
    DEVICE_SHIPMODE    = 2,
    DEVICE_POWER_RESET = 3,
}sdrv_ctrl;

//Delay time added to the taking action
typedef enum{
    ADD_10S_DELAY = 0,
    NO_DELAY      = 1,
}sdrv_dly;

//Charger forced both EN_ACDRV1 and EN_ACDRV2 to low
typedef enum{
    ACDRV_1_2_UP   = 0,
    ACDRV_1_2_DOWN = 1,
}dis_acdrv;

//Disable PFM in OTG mode
typedef enum{
    PFM_IN_OTG_ENABLE  = 0,
    PFM_IN_OTG_DISABLE = 1,
}pfm_otg_dis;

//Disable PFM in forward mode
typedef enum{
    PFM_IN_FORWARD_ENABLED  = 0,
    PFM_IN_FORWARD_DISABLED = 1,
}pfm_fwd_dis;

//When wake up the device from ship mode
typedef enum{
    WAKE_UP_FROM_SHIPMODE_1S   = 0,
    WAKE_UP_FROM_SHIPMODE_15MS = 1,
}wkup_dly;

//Disable BATFET LDO mode in pre-charge
typedef enum{
    DISABLE_BATFET_LDO_IN_PCHG_ENABLED  = 0,
    DISABLE_BATFET_LDO_IN_PCHG_DISABLED = 1,
}dis_ldo;

//Disable OOA in OTG mode
typedef enum{
    DISABLE_OOA_IN_OTG_ENABLED  = 0,
    DISABLE_OOA_IN_OTG_DISABLED = 1,
}dis_otg_ooa;

//Disable OOA in forward mode
typedef enum{
    DISABLE_OOA_IN_FORWARD_ENABLED  = 0,
    DISABLE_OOA_IN_FORWARD_DISABLED = 1,
}dis_fwd_ooa;

//ACFET2-RBFET2 gate driver control
typedef enum{
    TURN_OFF_ACDRV2 = 0,
    TURN_ON_ACDRV2  = 1,
}en_acdrv2; 

//ACFET1-RBFET1 gate driver control
typedef enum{
    TURN_OFF_ACDRV1 = 0,
    TURN_ON_ACDRV1  = 1,
}en_acdrv1;

//Switching frequency selection
typedef enum{
    FREQ_1_5_MHZ = 0,
    FREQ_750_KHZ  = 1,
}pwm_freq;

//Disable the STAT pin output
typedef enum{
    STAT_PIN_ENABLE  = 0,
    STAT_PIN_DISABLE = 1,
}dis_stat;

//Disable forward mode VSYS short hiccup protection
typedef enum{
    HICCUP_PROTECTION_ENABLE  = 0,
    HICCUP_PROTECTION_DISABLE = 1,
}dis_vsys_short;

//Disable OTG mode VOTG UVP hiccup protection
typedef enum{
    VOTG_UVP_HICCUP_PROTECTION_ENABLE  = 0,
    VOTG_UVP_HICCUP_PROTECTION_DISABLE = 1,
}dis_votg_uvp;

//Force VINDPM detection
typedef enum{
    DO_NOT_FORCE_VINDPM_DETECTION = 0,
    FORCE_VINDPM_DETECTION = 1,
}frc_vindpm_det;

//Enable IBUS_OCP in forward mode
typedef enum{
    IBUS_OCP_IN_FORWARD_DISABLE = 0,
    IBUS_OCP_IN_FORWARD_ENABLE = 1,
}en_ibus_ocp;

//Ship FET populated status
typedef enum{
    NO_SHIP_FET        = 0,
    SHIP_FET_POPULATED = 1,
}sfet_p;

//Battery discharging current regulation in OTG mode
typedef enum{
    IDSG_REGULATION_3A       = 0,
    IDSG_REGULATION_4A       = 1,
    IDSG_REGULATION_5A       = 2,
    IDSG_REGULATION_DISABLED = 3,
}ibat_reg_1;

//Enable the battery discharging current OCP
typedef enum{
    BAT_DSG_OVER_CURRENT_PROTECTION_DISABLED = 0,
    BAT_DSG_OVER_CURRENT_PROTECTION_ENABLED  = 1,
}en_batoc;

//To set the VINDPM as a percentage of the VBUS open circuit voltage when the VOC measurement is done
typedef enum{
    VINDPM_PERCANTAGE_OF_VOC_0_5625 = 0,
    VINDPM_PERCANTAGE_OF_VOC_0_625  = 1,
    VINDPM_PERCANTAGE_OF_VOC_0_6875 = 2,
    VINDPM_PERCANTAGE_OF_VOC_0_75   = 3,
    VINDPM_PERCANTAGE_OF_VOC_0_8125 = 4,
    VINDPM_PERCANTAGE_OF_VOC_0_875  = 5,
    VINDPM_PERCANTAGE_OF_VOC_0_9375 = 6,
    VINDPM_PERCANTAGE_OF_VOC_1      = 7,
}voc_pct_2;

//After the converter stops switching, the time delay before the VOC is measured
typedef enum{
    T_DELAY_AFTER_SWITCHING_50MS  = 0,
    T_DELAY_AFTER_SWITCHING_300MS = 1,
    T_DELAY_AFTER_SWITCHING_2S    = 2,
    T_DELAY_AFTER_SWITCHING_5S    = 3,
}voc_dly;

//The time interval two VBUS open circuit voltage measurements
typedef enum{
    T_DELAY_AFTER_SWITCHING_30S   = 0,
    T_DELAY_AFTER_SWITCHING_2MIN  = 1,
    T_DELAY_AFTER_SWITCHING_10MIN = 2,
    T_DELAY_AFTER_SWITCHING_30MIN = 3,
}voc_rate;

//Enable the MPPT to measure the VBUS open circuit voltage
typedef enum{
    DISABLE_MPPT = 0,
    ENABLE_MPPT  = 1,
}en_mppt;

//Thermal regulation thresholds
typedef enum{
    TEMP_60_C  = 0,
    TEMP_80_C  = 1,
    TEMP_100_C = 2,
    TEMP_120_C = 3,
}treg_1;

//Thermal shutdown thresholds
typedef enum{
    TEMP_SHUTDOWN_150_C = 0,
    TEMP_SHUTDOWN_130_C = 1,
    TEMP_SHUTDOWN_120_C = 2,
    TEMP_SHUTDOWN_85_C  = 3,
}t_shut_1;

//ACFET1 is off. Setting this bit to 1, the charger clears the EN_BACKUP bit to 0, sets DIS_ACDRV=0 and EN_ACDRV1=1 to turn on
typedef enum{
    ACFET1_IDLE              = 0,
    ACFET1_ON_IN_BACKUP_MODE = 1,
}bkup_acfet1_on;

//JEITA high temperature range (TWARN – THOT) charge voltage setting
typedef enum{
    CHARGE_SUSPEND         = 0,
    SET_VREG_TO_VREG_800mV = 1,
    SET_VREG_TO_VREG_600mV = 2,
    SET_VREG_TO_VREG_400mV = 3,
    SET_VREG_TO_VREG_300mV = 4,
    SET_VREG_TO_VREG_200mV = 5,
    SET_VREG_TO_VREG_100mV = 6,
    VREG_UNCHANGED         = 7,
} jeita_vset;

//JEITA high temperature range (TWARN – THOT) & (TCOLD – TCOOL) charge current setting
typedef enum{
    CHARGE_SUSPENDED            = 0,
    SET_ICHG_TO_20_PERC_OF_ICHG = 1,
    SET_ICHG_TO_40_PERC_OF_ICHG = 2,
    ICHG_UNCHANGED = 3,
} jeita_iseth_1;

typedef enum{
    PERCENT_71_1_AT_5_C  = 0,
    PERCENT_68_4_AT_10_C = 1,
    PERCENT_65_5_AT_15_C = 2,
    PERCENT_62_4_AT_20_C = 3,
} ts_cool_1;

typedef enum{
    PERCENT_48_4_AT_40_C = 0,
    PERCENT_44_8_AT_45_C = 1,
    PERCENT_41_2_AT_50_C = 2,
    PERCENT_37_7_AT_55_C = 3,
} ts_warm_1;

//OTG mode TS HOT temperature threshold
typedef enum{
    OTG_TS_HOT_55_C = 0,
    OTG_TS_HOT_60_C = 1,
    OTG_TS_HOT_65_C = 2,
    DISABLED_TRESHOLD = 3,
} bhot_1;

//OTG mode TS COLD temperature threshold
typedef enum{
    OTG_TS_COLD_MIN_10_C = 0,
    OTG_TS_COLD_MIN_20_C = 1,
} bcold;

typedef enum {
    ADC_CONTINUOUS = 0,
    ADC_ONE_SHOT = 1
} adc_rate;

// ADC Sample Resolution
typedef enum {
    ADC_15_BIT = 0,
    ADC_14_BIT = 1,
    ADC_13_BIT = 2,
    ADC_12_BIT = 3  // Not recommended
} adc_sample;

// ADC Averaging
typedef enum {
    ADC_SINGLE_VALUE = 0,
    ADC_RUNNING_AVERAGE = 1
} adc_avg;

// ADC Average Initialization
typedef enum {
    ADC_AVG_USE_EXISTING = 0,
    ADC_AVG_START_NEW = 1
} adc_avg_init;

//I2C Read/Write functions
HAL_StatusTypeDef BQ25798_ReadRegister(I2C_HandleTypeDef *hi2c, uint8_t reg_addr, uint8_t *data, uint8_t bytes);
HAL_StatusTypeDef BQ25798_WriteRegister(I2C_HandleTypeDef *hi2c, uint8_t reg_addr, uint8_t *data, uint8_t bytes);

//Register setting functions
void BQ25798_SetMinSystemVoltage(I2C_HandleTypeDef *hi2c, uint16_t voltage_mv);
HAL_StatusTypeDef BQ25798_SetChargeVoltageLimit(I2C_HandleTypeDef *hi2c, uint16_t voltage_mv);
HAL_StatusTypeDef BQ25798_SetChargeCurrentLimit(I2C_HandleTypeDef *hi2c, uint16_t current_ma);
HAL_StatusTypeDef BQ25798_SetInputCurrentLimit(I2C_HandleTypeDef *hi2c, uint16_t current_ma);
void BQ25798_SetPrechargeParam(I2C_HandleTypeDef *hi2c, uint16_t precharge_current_mA, vbat_lowv_t threshold);
void BQ25798_SetTerminationControl(I2C_HandleTypeDef *hi2c, uint16_t termination_current_mA, select expired_state, reset_state register_reset);
void BQ25798_SetRechargeControl(I2C_HandleTypeDef *hi2c, uint16_t recharge_threshold_mV, rechg_deglich_time deglich_time, cell_count number_of_cells);
void BQ25798_SetTimerControl(I2C_HandleTypeDef *hi2c, select tmr2x_state, fst_chg_tmr_ctrl fast_chg_timer_setting,select fast_chg_timer, select pre_chg_timer, 
                            select trickle_chg, top_off_tmr_ctrl top_off_timer_control);
void BQ25798_SetChargerControl_0(I2C_HandleTypeDef *hi2c, select backup_mode, select termination_enable, select hiz_enable, force_ico force_ico_start, 
                                select ico_enable, select charger_enable,force_ibatdis force_discharge_enable ,select auto_discharge_enable);
void BQ25798_SetChargerControl_1(I2C_HandleTypeDef *hi2c, wd watchdog_t,wd_reset watchdog_reset, vac_ovp_t vac_overvoltage_protection, vbus_bckp vbus_backup_threshold);
void BQ25798_SetChargerControl_2(I2C_HandleTypeDef *hi2c,sdrv_dly delay_to_device_mode, sdrv_ctrl device_mode, select HVDCP_enable, select nine_v_HVDCP_setting, 
                                select twelve_v_HVDCP_setting, select auto_indet_enable, f_indet force_indet);
void BQ25798_SetChargerControl_3(I2C_HandleTypeDef *hi2c, dis_fwd_ooa disable_forward_ooa, dis_otg_ooa disable_otg_ooa, dis_ldo disable_LDO, wkup_dly wakeup_shipmode_delay, 
                                pfm_fwd_dis disable_PFM_in_forward, pfm_otg_dis disable_PFM_in_OTG, select OTG_mode, dis_acdrv state_of_ACDRV);
void BQ25798_SetChargerControl_4(I2C_HandleTypeDef *hi2c, en_ibus_ocp ibus_ocp_in_forward, frc_vindpm_det vindpm_detection, dis_votg_uvp VOTG_UVP_hiccup_protection,
                                dis_vsys_short VSYS_short_hiccup_protection, dis_stat STAT_pin, pwm_freq switching_frequency, en_acdrv1 ACDRV1_state, en_acdrv2 ACDRV2_state);
void BQ25798_SetChargerControl_5(I2C_HandleTypeDef *hi2c, select battery_discharge_ocp, select external_ilim_hiz, select internal_iindpm, ibat_reg_1 ibat_discharge_regulation, 
                                select ibat_discharge_sensing, sfet_p ship_fet_population);
void BQ25798_SetMPPTControl(I2C_HandleTypeDef *hi2c, en_mppt mppt_state, voc_rate voc_measurement_rate, voc_dly voc_measurement_delay, voc_pct_2 voc_percentage_of_voc);
void BQ25798_SetTemperatureControl(I2C_HandleTypeDef *hi2c, bkup_acfet1_on acfet1_activation_in_backup, select VAC2_pull_down_resistor, select VAC1_pull_down_resistor, 
                                  select VBUS_pull_down_resistor, t_shut_1 thermal_shutdown, treg_1 thermal_regulation_t);
void BQ25798_SetNTCCONTROL_0(I2C_HandleTypeDef *hi2c, jeita_iseth_1 low_temperature_current_setting, jeita_iseth_1 high_temperature_current_setting, jeita_vset high_temperature_range_charge_voltage);
void BQ25798_SetNTCCONTROL_1(I2C_HandleTypeDef *hi2c, select ts_feedback, bcold ts_cold_threshold, bhot_1 ts_hot_threshold, ts_warm_1 ts_warm_percentage, ts_cool_1 ts_cool_percentage);
void BQ25798_SetADCControl(I2C_HandleTypeDef *hi2c, select enable, adc_rate rate, adc_sample sample, adc_avg avg, adc_avg_init avg_init);


HAL_StatusTypeDef BQ25798_DeviceCheck(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef BQ25798_FaultCheck(I2C_HandleTypeDef *hi2c, uint8_t *fault_status_0, uint8_t *fault_status_1);
HAL_StatusTypeDef BQ25798_ChargerStatus(I2C_HandleTypeDef *hi2c, uint8_t *ChargerStatus0 ,uint8_t *ChargerStatus1, uint8_t *ChargerStatus2, uint8_t *ChargerStatus3, uint8_t *ChargerStatus4);
HAL_StatusTypeDef BQ25798_ChargerFlags(I2C_HandleTypeDef *hi2c, uint8_t *charger_flag_0, uint8_t *charger_flag_1, uint8_t *charger_flag_2, uint8_t *charger_flag_3);
HAL_StatusTypeDef BQ25798_FaultFlags(I2C_HandleTypeDef *hi2c, uint8_t *fault_flags_0, uint8_t *fault_flags_1);
HAL_StatusTypeDef BQ25798_VerifyConfiguration(I2C_HandleTypeDef *hi2c);
#endif
