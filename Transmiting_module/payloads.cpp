#include "payloads.h"
#include "rs485_dock.h"
#include <Arduino.h>


// POWER MANAGEMENT MODULE PAYLOADS
void payload_01(){
  uint8_t payload[11];

  payload[0] = 0x01;
  payload[1] = (cell1_voltage >> 8) & 0xFF;
  payload[2] = cell1_voltage & 0xFF;
  payload[3] = (cell2_voltage >> 8) & 0xFF;
  payload[4] = cell2_voltage & 0xFF;
  payload[5] = (cell3_voltage >> 8) & 0xFF;
  payload[6] = cell3_voltage & 0xFF;
  payload[7] = (pack_temp_BMS >> 8) & 0xFF;
  payload[8] = pack_temp_BMS & 0xFF;
  payload[9] = (pack_temp_Charger >> 8) & 0xFF;
  payload[10] = pack_temp_Charger & 0xFF;

  api.lorawan.send(11, payload, 11);
}

void payload_11(){
  uint8_t  payload[7];

  payload[0] = 0x11;
  payload[1] = (vac1_voltage >> 8) & 0xFF;
  payload[2] = vac1_voltage & 0xFF;
  payload[3] = (gen_sys_status >> 8) & 0xFF;
  payload[4] = gen_sys_status & 0xFF;
  payload[5] = (current_pack >> 8) & 0xFF;
  payload[6] = current_pack & 0xFF;

  api.lorawan.send(7, payload, 7);
}

void payload_21(){ //fault check
  uint8_t  payload[3];

  payload[0] = 0x21;
  payload[1] = (fault_flags[0] >> 8) & 0xFF;
  payload[2] = fault_flags[0] & 0xFF;

  api.lorawan.send(3, payload, 3);
}

void payload_31(){ //fault flags
  uint8_t  payload[3];

  payload[0] = 0x31;
  payload[1] = (fault_flags[1] >> 8) & 0xFF;
  payload[2] = fault_flags[1] & 0xFF;
  
  api.lorawan.send(3, payload, 3);
}

void payload_41(){ //charger flgs
  uint8_t  payload[5];

  payload[0] = 0x41;
  payload[1] = (charger_flags[0] >> 8) & 0xFF;
  payload[2] = charger_flags[0] & 0xFF;
  payload[3] = (charger_flags[1] >> 8) & 0xFF;
  payload[4] = charger_flags[1] & 0xFF;

  api.lorawan.send(5, payload, 3);
}

void payload_51(){ //BMS safty alert
  uint8_t  payload[4];

  payload[0] = 0x51;
  payload[1] = (bms_safety[0] >> 8) & 0xFF;
  payload[2] = bms_safety[0] & 0xFF;
  payload[3] = (bms_safety[1]  >> 8) & 0xFF;

  api.lorawan.send(4, payload, 3);
} 

void payload_61(){ //BMS safty status
  uint8_t  payload[4];

  payload[0] = 0x61;
  payload[1] = bms_safety[1] & 0xFF;
  payload[2] = (bms_safety[2]  >> 8) & 0xFF;
  payload[3] = bms_safety[2] & 0xFF;

  api.lorawan.send(4, payload, 4);
} 

void payload_71(){ //BMS PF alerts
  uint8_t  payload[5];

  payload[0] = 0x71;
  payload[1] = (bms_pf_alert[0]  >> 8) & 0xFF;
  payload[2] = bms_pf_alert[0] & 0xFF;
  payload[3] = (bms_pf_alert[1]  >> 8) & 0xFF;
  payload[4] = bms_pf_alert[1] & 0xFF;

  api.lorawan.send(5, payload, 5);
} 
void payload_81(){ //BMS PF status
  uint8_t  payload[5];

  payload[0] = 0x81;
  payload[1] = (bms_pf_status[0]  >> 8) & 0xFF;
  payload[2] = bms_pf_status[0] & 0xFF;
  payload[3] = (bms_pf_status[1]  >> 8) & 0xFF;
  payload[4] = bms_pf_status[1] & 0xFF;

  api.lorawan.send(5, payload, 5);
} 


// DATA GATHERING MODULE PAYLOADS

void payload_02(){
  uint8_t payload[7];

  payload[0] = 0x02;
  payload[1] = (temp_SHT45 >> 8) & 0xFF;
  payload[2] = temp_SHT45 & 0xFF;
  payload[3] = (humidity_SHT45 >> 8) & 0xFF;
  payload[4] = humidity_SHT45 & 0xFF;
  payload[5] = (temp_BMP390 >> 8) & 0xFF;
  payload[6] = temp_BMP390 & 0xFF;

  api.lorawan.send(7, payload, 7);
}


void payload_12(){
  uint8_t payload[11];

  payload[0] = 0x12;
  payload[1] = (lux_VEML7700 >> 24) & 0xFF;
  payload[2] = (lux_VEML7700 >> 16) & 0xFF;
  payload[3] = (lux_VEML7700 >> 8) & 0xFF;
  payload[4] = lux_VEML7700 & 0xFF;
  payload[5] = (whiteRatio_VEML7700 >> 8) & 0xFF;
  payload[6] = whiteRatio_VEML7700 & 0xFF;
  payload[7] = (pressure_BMP390 >> 24) & 0xFF;
  payload[8] = (pressure_BMP390 >> 16) & 0xFF;
  payload[9] = (pressure_BMP390 >> 8) & 0xFF;
  payload[10] = pressure_BMP390 & 0xFF;

  api.lorawan.send(11, payload, 11);
}



