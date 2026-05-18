#include "rs485dock.h"
#include "modbus_rtu.h"
#include "init.h"
#include "control.h"
#include"main.h"


// === SENSOR DATA REGISTERS (0-7) ===
#define REG_CELL_VOLTAGE_1       0
#define REG_CELL_VOLTAGE_2       1
#define REG_CELL_VOLTAGE_3       2
#define REG_BMS_CURRENT          3
#define REG_BMS_CELL_TEMPERATURE 4
#define REG_CHG_CELL_TEMPERATURE 5
#define REG_VAC1_VOLTAGE         6
#define REG_SYS_STATUS           7

// === STATUS FLAGS REGISTERS (8-18) ===
#define REG_FAULT_FLAGS_0        8   // [fault_check_0 | fault_check_1]
#define REG_FAULT_FLAGS_1        9   // [fault_flag_0 | fault_flag_1]

#define REG_CHARGER_FLAGS_0      10  // [charger_flag_0 | charger_flag_1]
#define REG_CHARGER_FLAGS_1      11  // [charger_flag_2 | charger_flag_3]

#define REG_SAFETY_ALERT         12  // [safety_alert_a | safety_alert_b]
#define REG_SAFETY_ALERT_C       13  // [safety_alert_c | safety_status_a]
#define REG_SAFETY_STATUS        14  // [safety_status_b | safety_status_c]

#define REG_PF_ALERT_0           15  // [pf_alert_a | pf_alert_b]
#define REG_PF_ALERT_1           16  // [pf_alert_c | pf_alert_d]
#define REG_PF_STATUS_0          17  // [pf_status_a | pf_status_b]
#define REG_PF_STATUS_1          18  // [pf_status_c | pf_status_d]


void RS485_Init(UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim)
{
    Modbus_Init(huart, htim);
}

// Update sensor data (call frequently)
void RS485_UpdateRegisters(void)
{
    // Send raw values - NO SCALING!
    Modbus_SetRegisterU16(REG_CELL_VOLTAGE_1, g_system_data.cell1_voltage);
    Modbus_SetRegisterU16(REG_CELL_VOLTAGE_2, g_system_data.cell2_voltage);
    Modbus_SetRegisterU16(REG_CELL_VOLTAGE_3, g_system_data.cell3_voltage);
    Modbus_SetRegisterU16(REG_BMS_CURRENT, (uint16_t)g_system_data.current_pack);
    Modbus_SetRegisterU16(REG_BMS_CELL_TEMPERATURE, g_system_data.pack_temperature_BMS);
    Modbus_SetRegisterU16(REG_CHG_CELL_TEMPERATURE, g_system_data.pack_temperature_Charger);
    Modbus_SetRegisterU16(REG_VAC1_VOLTAGE, g_system_data.vac1_voltage);
    Modbus_SetRegisterU16(REG_SYS_STATUS, g_system_data.gen_sys_status);
}

// Update status flags (call less often, e.g., every 5-10 seconds)
void RS485_UpdateFlags(void)
{
    // Pack 2 uint8 values into each uint16 register
    
    // Fault flags - use g_fault_check
    Modbus_SetRegisterU16(REG_FAULT_FLAGS_0, 
        (g_fault_check.fault_check_0 << 8) | g_fault_check.fault_check_1);
    Modbus_SetRegisterU16(REG_FAULT_FLAGS_1, 
        (g_fault_check.fault_flag_0 << 8) | g_fault_check.fault_flag_1);
    
    // Charger flags - use g_charger_status
    Modbus_SetRegisterU16(REG_CHARGER_FLAGS_0, 
        (g_charger_status.charger_flag_0 << 8) | g_charger_status.charger_flag_1);
    Modbus_SetRegisterU16(REG_CHARGER_FLAGS_1, 
        (g_charger_status.charger_flag_2 << 8) | g_charger_status.charger_flag_3);
    
    // BMS Safety alerts - use g_bms_status
    Modbus_SetRegisterU16(REG_SAFETY_ALERT, 
        (g_bms_status.safety_alert_a << 8) | g_bms_status.safety_alert_b);
    Modbus_SetRegisterU16(REG_SAFETY_ALERT_C, 
        (g_bms_status.safety_alert_c << 8) | g_bms_status.safety_status_a);
    Modbus_SetRegisterU16(REG_SAFETY_STATUS, 
        (g_bms_status.safety_status_b << 8) | g_bms_status.safety_status_c);
    
    // BMS PF alerts
    Modbus_SetRegisterU16(REG_PF_ALERT_0, 
        (g_bms_status.pf_alert_a << 8) | g_bms_status.pf_alert_b);
    Modbus_SetRegisterU16(REG_PF_ALERT_1, 
        (g_bms_status.pf_alert_c << 8) | g_bms_status.pf_alert_d);
    
    // BMS PF status
    Modbus_SetRegisterU16(REG_PF_STATUS_0, 
        (g_bms_status.pf_status_a << 8) | g_bms_status.pf_status_b);
    Modbus_SetRegisterU16(REG_PF_STATUS_1, 
        (g_bms_status.pf_status_c << 8) | g_bms_status.pf_status_d);
}

void RS485_Process(void)
{
    Modbus_Process();
}