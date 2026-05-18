#ifndef RS485_DOCK_H
#define RS485_DOCK_H

#include <Arduino.h>

// ========== WEATHER STATION DATA (Slave 2) ==========
// 8 registers total
extern uint16_t temp_SHT45;         // Reg 0: Temperature × 100
extern uint16_t humidity_SHT45;     // Reg 1: Humidity × 100
extern uint32_t lux_VEML7700;       // Reg 2-3: Lux (direct)
extern uint16_t whiteRatio_VEML7700; // Reg 4: White ratio × 1000
extern uint16_t temp_BMP390;        // Reg 5: Temperature × 100
extern uint32_t pressure_BMP390;    // Reg 6-7: Pressure Pa (direct)

// ========== POWER BOARD DATA (Slave 1) ==========
// 19 registers total
extern uint16_t cell1_voltage;      // Reg 0: mV
extern uint16_t cell2_voltage;      // Reg 1: mV
extern uint16_t cell3_voltage;      // Reg 2: mV
extern uint16_t current_pack;       // Reg 3: mA (as uint16, cast to int16)
extern uint16_t pack_temp_BMS;      // Reg 4: Temperature × 10
extern uint16_t pack_temp_Charger;  // Reg 5: Raw ADC
extern uint16_t vac1_voltage;       // Reg 6: mV
extern uint16_t gen_sys_status;     // Reg 7: Status flags
extern uint16_t fault_flags[2];     // Reg 8-9: Fault flags
extern uint16_t charger_flags[2];   // Reg 10-11: Charger flags
extern uint16_t bms_safety[3];      // Reg 12-14: BMS safety
extern uint16_t bms_pf_alert[2];    // Reg 15-16: PF alerts
extern uint16_t bms_pf_status[2];   // Reg 17-18: PF status

// Functions to update global variables
void Weather_UpdateData();
void Power_UpdateData();
void Power_UpdateFaults();

#endif