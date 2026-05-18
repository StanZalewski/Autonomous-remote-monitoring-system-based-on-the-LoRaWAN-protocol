#include "bq25798Charger.h"
#include "stm32g0xx_hal.h"
#include "stm32g0xx_hal_i2c.h"
#include <stdint.h>


// Reading data from BQ25798 charger IC
HAL_StatusTypeDef BQ25798_ReadRegister(I2C_HandleTypeDef *hi2c, uint8_t reg_addr, uint8_t *data, uint8_t bytes)
{
    return HAL_I2C_Mem_Read(
        hi2c,                           
        BQ25798_I2C_ADDRESS,               
        reg_addr,                       
        I2C_MEMADD_SIZE_8BIT,           
        data,                           
        bytes,                              
        BQ25798_I2C_TIMEOUT             
    );
}

// Writing data to BQ25798 charger IC
HAL_StatusTypeDef BQ25798_WriteRegister(I2C_HandleTypeDef *hi2c, uint8_t reg_addr, uint8_t *data, uint8_t bytes)
{
    return HAL_I2C_Mem_Write(
        hi2c,                           
        BQ25798_I2C_ADDRESS,               
        reg_addr,                       
        I2C_MEMADD_SIZE_8BIT,           
        data,                          
        bytes,                              
        BQ25798_I2C_TIMEOUT             
    );
}

//REG00 Minimal System Voltage
void BQ25798_SetMinSystemVoltage(I2C_HandleTypeDef *hi2c, uint16_t voltage_mv)
{
    uint8_t reg_value;
    reg_value = (voltage_mv - 2500) / 250;
    BQ25798_WriteRegister(hi2c, 0x00, &reg_value,1);
}

//REG01 Charge Voltage Limit
HAL_StatusTypeDef BQ25798_SetChargeVoltageLimit(I2C_HandleTypeDef *hi2c, uint16_t voltage_mv)
{
    uint8_t data[2];
    uint16_t reg_value = (voltage_mv) / 10;
    
    data[0] = (reg_value >> 8) & 0x07;
    data[1] = reg_value & 0xFF;

    return BQ25798_WriteRegister(hi2c, 0x01, data, 2);
}

//REG03 Charge Current Limit
HAL_StatusTypeDef BQ25798_SetChargeCurrentLimit(I2C_HandleTypeDef *hi2c, uint16_t current_ma)
{   
    uint8_t data[2];
    uint16_t reg_value = (current_ma) / 10;
    
    data[0] = (reg_value >> 8) & 0x07;
    data[1] = reg_value & 0xFF;

    return BQ25798_WriteRegister(hi2c, 0x03, data, 2);
}

//REG06 Input Current Limit
HAL_StatusTypeDef BQ25798_SetInputCurrentLimit(I2C_HandleTypeDef *hi2c, uint16_t current_ma)
{   
    uint8_t data[2];
    uint16_t reg_value = (current_ma) / 10;
    
    data[0] = (reg_value >> 8) & 0x01;
    data[1] = reg_value & 0xFF;

    return BQ25798_WriteRegister(hi2c, 0x06, data, 2);
}

//REG08 Precharge Control
void BQ25798_SetPrechargeParam(I2C_HandleTypeDef *hi2c, uint16_t precharge_current_mA, vbat_lowv_t threshold)
{
    uint8_t reg_value;
    uint8_t iprechg = precharge_current_mA / 40;

    reg_value = ((threshold & 0x03) << 6) | (iprechg & 0x3F);
    BQ25798_WriteRegister(hi2c, 0x08, &reg_value,1);
}

//REG09 Termination Control
void BQ25798_SetTerminationControl(I2C_HandleTypeDef *hi2c, uint16_t termination_current_mA, select expired_state, reset_state register_reset)
{
    uint8_t reg_value;
    uint8_t iterm = termination_current_mA / 40;

    reg_value = ((register_reset & 0x01) << 6) | ((expired_state & 0x01) << 5) | (iterm & 0x1F);
    BQ25798_WriteRegister(hi2c, 0x09, &reg_value,1);
}

//REG0A Recharge Control
void BQ25798_SetRechargeControl(I2C_HandleTypeDef *hi2c, uint16_t recharge_threshold_mV, rechg_deglich_time deglich_time, cell_count number_of_cells)
{
    uint8_t reg_value;
    uint8_t vrechg = (recharge_threshold_mV - 50) / 50;

    reg_value = ((number_of_cells & 0x03) << 6) | ((deglich_time & 0x03) << 4) | (vrechg & 0x0F);
    BQ25798_WriteRegister(hi2c, 0x0A, &reg_value,1);
}

//REG0E Timer Control
void BQ25798_SetTimerControl(I2C_HandleTypeDef *hi2c, select tmr2x_state, fst_chg_tmr_ctrl fast_chg_timer_setting,select fast_chg_timer, select pre_chg_timer, select trickle_chg, top_off_tmr_ctrl top_off_timer_control)
{
    uint8_t reg_value;

    reg_value = ((top_off_timer_control & 0x03) << 6) | ((trickle_chg & 0x01) << 5) | ((pre_chg_timer & 0x01) <<4) | ((fast_chg_timer & 0x01) <<3) | ((fast_chg_timer_setting & 0x03) <<1) | (tmr2x_state & 0x01);
    BQ25798_WriteRegister(hi2c, 0x0E, &reg_value,1);
}

//REG0F Charger Control 0
void BQ25798_SetChargerControl_0(I2C_HandleTypeDef *hi2c, select backup_mode, select termination_enable, select hiz_enable, force_ico force_ico_start, select ico_enable, select charger_enable,force_ibatdis force_discharge_enable ,select auto_discharge_enable)
{
    uint8_t reg_value;

    reg_value = ((auto_discharge_enable & 0x01) << 7) | ((force_discharge_enable & 0x01) << 6) | ((charger_enable & 0x01) << 5) | ((ico_enable & 0x01) <<4) | ((force_ico_start & 0x01) <<3) | ((hiz_enable & 0x01) <<2) | ((termination_enable & 0x01) <<1) | ((backup_mode & 0x01));
    BQ25798_WriteRegister(hi2c, 0x0F, &reg_value,1);
}

//REG10 Charger Control 1
void BQ25798_SetChargerControl_1(I2C_HandleTypeDef *hi2c, wd watchdog_t,wd_reset watchdog_reset, vac_ovp_t vac_overvoltage_protection, vbus_bckp vbus_backup_threshold)
{
    uint8_t reg_value;

    reg_value = ((vbus_backup_threshold & 0x03) << 6) | ((vac_overvoltage_protection & 0x03) << 4) | ((watchdog_reset & 0x01) << 3) | (watchdog_t & 0x07);
    BQ25798_WriteRegister(hi2c, 0x10, &reg_value,1);
}

//REG11 Charger Control 2
void BQ25798_SetChargerControl_2(I2C_HandleTypeDef *hi2c,sdrv_dly delay_to_device_mode, sdrv_ctrl device_mode, select HVDCP_enable, select nine_v_HVDCP_setting, select twelve_v_HVDCP_setting, select auto_indet_enable, f_indet force_indet)
{
    uint8_t reg_value;

    reg_value = ((force_indet & 0x01) <<7) | ((auto_indet_enable & 0x01) <<6) | ((twelve_v_HVDCP_setting & 0x01) <<5) | ((nine_v_HVDCP_setting & 0x01) <<4) | ((HVDCP_enable & 0x01) <<3) | ((device_mode & 0x03) <<1) | (delay_to_device_mode & 0x01);
    BQ25798_WriteRegister(hi2c, 0x11, &reg_value,1);
}

//REG12 Charger Control 3
void BQ25798_SetChargerControl_3(I2C_HandleTypeDef *hi2c, dis_fwd_ooa disable_forward_ooa, dis_otg_ooa disable_otg_ooa, dis_ldo disable_LDO, wkup_dly wakeup_shipmode_delay, pfm_fwd_dis disable_PFM_in_forward, pfm_otg_dis disable_PFM_in_OTG, select OTG_mode, dis_acdrv state_of_ACDRV)
{
    uint8_t reg_value;

    reg_value = ((state_of_ACDRV & 0x01) << 7) | ((OTG_mode & 0x01) << 6) | ((disable_PFM_in_OTG & 0x01) <<5) | ((disable_PFM_in_forward & 0x01) <<4) | ((wakeup_shipmode_delay & 0x01) <<3) | ((disable_LDO & 0x01) <<2) | ((disable_otg_ooa & 0x01) <<1) | (disable_forward_ooa & 0x01);
    BQ25798_WriteRegister(hi2c, 0x12, &reg_value,1);
}

//REG13 Charger Control 4
void BQ25798_SetChargerControl_4(I2C_HandleTypeDef *hi2c, en_ibus_ocp ibus_ocp_in_forward, frc_vindpm_det vindpm_detection, dis_votg_uvp VOTG_UVP_hiccup_protection, dis_vsys_short VSYS_short_hiccup_protection, dis_stat STAT_pin, pwm_freq switching_frequency, en_acdrv1 ACDRV1_state, en_acdrv2 ACDRV2_state)
{
    uint8_t reg_value;

    reg_value = ((ACDRV2_state & 0x01) <<7) | ((ACDRV1_state & 0x01) <<6) | ((switching_frequency & 0x01) <<5) | ((STAT_pin & 0x01) <<4) | ((VSYS_short_hiccup_protection & 0x01) <<3) | ((VOTG_UVP_hiccup_protection & 0x01) <<2) | ((vindpm_detection & 0x01) <<1) | (ibus_ocp_in_forward & 0x01);
    BQ25798_WriteRegister(hi2c, 0x13, &reg_value,1);
}

//REG14 Charger Control 5
void BQ25798_SetChargerControl_5(I2C_HandleTypeDef *hi2c, select battery_discharge_ocp, select external_ilim_hiz, select internal_iindpm, ibat_reg_1 ibat_discharge_regulation, select ibat_discharge_sensing, sfet_p ship_fet_population)
{
    uint8_t reg_value;

    reg_value = ((ship_fet_population & 0x01) <<7) | ((ibat_discharge_sensing & 0x01) <<5) | ((ibat_discharge_regulation & 0x03) <<3) | ((internal_iindpm & 0x01) <<2) | ((external_ilim_hiz & 0x01) <<1) | (battery_discharge_ocp & 0x01);
    BQ25798_WriteRegister(hi2c, 0x14, &reg_value,1);
}

//REG15 MPPT Control
void BQ25798_SetMPPTControl(I2C_HandleTypeDef *hi2c, en_mppt mppt_state, voc_rate voc_measurement_rate, voc_dly voc_measurement_delay, voc_pct_2 voc_percentage_of_voc)
{
    uint8_t reg_value;

    reg_value = ((voc_percentage_of_voc & 0x07) <<5) | ((voc_measurement_delay & 0x03) <<3) | ((voc_measurement_rate & 0x03) <<1) | (mppt_state & 0x01);
    BQ25798_WriteRegister(hi2c, 0x15, &reg_value,1);
}

//REG16 Temperature Control Register
void BQ25798_SetTemperatureControl(I2C_HandleTypeDef *hi2c, bkup_acfet1_on acfet1_activation_in_backup, select VAC2_pull_down_resistor, select VAC1_pull_down_resistor, select VBUS_pull_down_resistor, t_shut_1 thermal_shutdown, treg_1 thermal_regulation_t)
{
    uint8_t reg_value;

    reg_value = ((thermal_regulation_t & 0x03) <<6) | ((thermal_shutdown & 0x03) <<4) | ((VBUS_pull_down_resistor & 0x01) <<3) | ((VAC1_pull_down_resistor & 0x01) <<2) | ((VAC2_pull_down_resistor & 0x01) <<1) | (acfet1_activation_in_backup & 0x01);
    BQ25798_WriteRegister(hi2c, 0x16, &reg_value,1);
}

//REG17 NTC Control 0
void BQ25798_SetNTCCONTROL_0(I2C_HandleTypeDef *hi2c, jeita_iseth_1 low_temperature_current_setting, jeita_iseth_1 high_temperature_current_setting, jeita_vset high_temperature_range_charge_voltage)
{
    uint8_t reg_value;

    reg_value = ((high_temperature_range_charge_voltage & 0x07) <<5) | ((high_temperature_current_setting & 0x03 ) <<3) | ((low_temperature_current_setting & 0x03 ) <<1);
    BQ25798_WriteRegister(hi2c, 0x17, &reg_value,1);
}

//REG178 NTC Control 1
void BQ25798_SetNTCCONTROL_1(I2C_HandleTypeDef *hi2c, select ts_feedback, bcold ts_cold_threshold, bhot_1 ts_hot_threshold, ts_warm_1 ts_warm_percentage, ts_cool_1 ts_cool_percentage)
{
    uint8_t reg_value;

    reg_value = ((ts_cool_percentage & 0x03) <<6) | ((ts_warm_percentage & 0x03) <<4) | ((ts_hot_threshold & 0x03) <<2) | ((ts_cold_threshold & 0x01) <<1) | (ts_feedback & 0x01);
    BQ25798_WriteRegister(hi2c, 0x18, &reg_value,1);
}

void BQ25798_SetADCControl(I2C_HandleTypeDef *hi2c, select enable, adc_rate rate, adc_sample sample, adc_avg avg, adc_avg_init avg_init)
{
    uint8_t reg_value;

    reg_value = ((enable & 0x01) << 7) | ((rate & 0x01) << 6) | ((sample & 0x03) << 4) | ((avg & 0x01) << 3) | ((avg_init & 0x01) << 2);

    BQ25798_WriteRegister(hi2c, 0x2E, &reg_value, 1);
}



// Device Check
HAL_StatusTypeDef BQ25798_DeviceCheck(I2C_HandleTypeDef *hi2c){
    uint8_t part_info;
    HAL_StatusTypeDef status;
    
    status = BQ25798_ReadRegister(hi2c, 0x48, &part_info, 1);
    
    if (status != HAL_OK) {
        return status;  
    }

    uint8_t part_number = (part_info >> 3) & 0x07;  
    
    if (part_number == 0x03) {
        return HAL_OK; 
    } else {
        return HAL_ERROR;
    }
}

HAL_StatusTypeDef BQ25798_FaultCheck(I2C_HandleTypeDef *hi2c, uint8_t *fault_status_0, uint8_t *fault_status_1){
    HAL_StatusTypeDef status;

    status = BQ25798_ReadRegister(hi2c, 0x20, fault_status_0, 1);
    if (status != HAL_OK) return status;
    
    status = BQ25798_ReadRegister(hi2c, 0x21, fault_status_1, 1);
    if (status != HAL_OK) return status;

    return HAL_OK;
}

HAL_StatusTypeDef BQ25798_ChargerStatus(I2C_HandleTypeDef *hi2c, uint8_t *ChargerStatus0 ,uint8_t *ChargerStatus1, uint8_t *ChargerStatus2, uint8_t *ChargerStatus3, uint8_t *ChargerStatus4)
    {
    HAL_StatusTypeDef status;
    
    status = BQ25798_ReadRegister(hi2c, 0x1B, ChargerStatus0, 1);
    if (status != HAL_OK) return status;
    
    status = BQ25798_ReadRegister(hi2c, 0x1C, ChargerStatus1, 1);
    if (status != HAL_OK) return status;
    
    status = BQ25798_ReadRegister(hi2c, 0x1D, ChargerStatus2, 1);
    if (status != HAL_OK) return status;
    
    status = BQ25798_ReadRegister(hi2c, 0x1E, ChargerStatus3, 1);
    if (status != HAL_OK) return status;
    
    status = BQ25798_ReadRegister(hi2c, 0x1F, ChargerStatus4, 1);
    if (status != HAL_OK) return status;
    
    return HAL_OK;
}
HAL_StatusTypeDef BQ25798_ChargerFlags(I2C_HandleTypeDef *hi2c, uint8_t *charger_flag_0, uint8_t *charger_flag_1, uint8_t *charger_flag_2, uint8_t *charger_flag_3){
    HAL_StatusTypeDef status;

    status = BQ25798_ReadRegister(hi2c, 0x22, charger_flag_0, 1);
    if (status != HAL_OK) return status;
    
    status = BQ25798_ReadRegister(hi2c, 0x23, charger_flag_1, 1);
    if (status != HAL_OK) return status;
    
    status = BQ25798_ReadRegister(hi2c, 0x24, charger_flag_2, 1);
    if (status != HAL_OK) return status;
    
    status = BQ25798_ReadRegister(hi2c, 0x25, charger_flag_3, 1);
    if (status != HAL_OK) return status;
    return HAL_OK;
}

HAL_StatusTypeDef BQ25798_FaultFlags(I2C_HandleTypeDef *hi2c, uint8_t *fault_flags_0, uint8_t *fault_flags_1){
    HAL_StatusTypeDef status;
    
    // Step 1: Read current fault flags
    status = BQ25798_ReadRegister(hi2c, 0x26, fault_flags_0, 1);
    if (status != HAL_OK) return status;
    
    status = BQ25798_ReadRegister(hi2c, 0x27, fault_flags_1, 1);
    if (status != HAL_OK) return status;
    
    return HAL_OK;
}

HAL_StatusTypeDef BQ25798_VerifyConfiguration(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;
    uint8_t reg_data[2];
    uint16_t readback_value;
    
    status = BQ25798_ReadRegister(hi2c, 0x01, reg_data, 2);
    if (status != HAL_OK) return HAL_ERROR;
    readback_value = (((reg_data[0] & 0x07) << 8 | reg_data[1]) * 10);
    if (readback_value != 10800) return HAL_ERROR;
    
    status = BQ25798_ReadRegister(hi2c, 0x03, reg_data, 2);
    if (status != HAL_OK) return HAL_ERROR;
    readback_value = ((reg_data[0] & 0x07) << 8 | reg_data[1]) * 10;
    if (readback_value != 1300) return HAL_ERROR;
    
    status = BQ25798_ReadRegister(hi2c, 0x06, reg_data, 2);
    if (status != HAL_OK) return HAL_ERROR;
    readback_value = ((reg_data[0] & 0x01) << 8 | reg_data[1]) * 10;
    if (readback_value != 1670) return HAL_ERROR;
    
    status = BQ25798_ReadRegister(hi2c, 0x00, reg_data, 1);
    if (status != HAL_OK) return HAL_ERROR;
    readback_value = (reg_data[0] * 250) + 2500;
    if (readback_value != 8500) return HAL_ERROR;
    
    status = BQ25798_ReadRegister(hi2c, 0x10, reg_data, 1);
    if (status != HAL_OK) return HAL_ERROR;
    if ((reg_data[0] & 0x07) != 0x06) return HAL_ERROR;
    
    return HAL_OK;
}
