#include "loraWAN_dock.h"
#include "rs485_dock.h"
#include "payloads.h"
#include <Arduino.h>



uint8_t payload_01_time = 0;
uint8_t payload_11_time = 0;
uint8_t payload_21_time = 0;
uint8_t payload_31_time = 0;
uint8_t payload_41_time = 0;
uint8_t payload_51_time = 0;
uint8_t payload_61_time = 0;
uint8_t payload_71_time = 0;
uint8_t payload_81_time = 0;
uint8_t payload_02_time = 0;
uint8_t payload_12_time = 0;

static uint16_t charger_flags_prev[2] = {0, 0};


void loraInit()
{
  api.lorawan.band.set(4);//EUROPE 868MHz band

  api.lorawan.deui.set(DEVEUI, 8);
  api.lorawan.appeui.set(APPEUI, 8);
  api.lorawan.appkey.set(APPKEY, 16);

  api.lorawan.njm.set(1); // OTTA JOIN TYPE
  api.lorawan.cfm.set(0); //No ACK from the Gateway side normaly
  api.lorawan.rety.set(0);
  api.lorawan.adr.set(true);// Automatic Data rate control
  api.lorawan.deviceClass.set(0);//Class A for now.

  if (api.lorawan.join() == true){
    Serial.println("Join network!");
  }
  else{
    Serial.println("Join failed!");
    Serial.println("Trying to join again in 10s");
    delay(10000);
    loraInit();
  }
}




void sendLoRaCallback(void *) {

    // Increment all timers
    payload_01_time++;
    payload_11_time++;
    payload_02_time++;
    payload_12_time++;
    
    // Update fault/flag data from Modbus
    Power_UpdateFaults();
    
    // Priority 1: Weather messages (most frequent)
    if (payload_02_time >= 5) {
        Weather_UpdateData();
        payload_02();
        payload_02_time = 0;
    }
    else if (payload_12_time >= 10) {
        Weather_UpdateData();
        payload_12();
        payload_12_time = 0;
    }
    // Priority 2: Fault/Alert messages (send if non-zero - they auto-clear)
    else if (fault_flags[0] != 0) {
        payload_21();
    }
    else if (fault_flags[1] != 0) {
        payload_31();
    }
    else if (bms_safety[0] != 0 || (bms_safety[1] >> 8) != 0) {
        payload_51();  // BMS safety alert (upper part)
    }
    else if ((bms_safety[1] & 0xFF) != 0 || bms_safety[2] != 0) {
        payload_61();  // BMS safety status (lower part)
    }
    else if (bms_pf_alert[0] != 0 || bms_pf_alert[1] != 0) {
        payload_71();  // BMS PF alerts
    }
    else if (bms_pf_status[0] != 0 || bms_pf_status[1] != 0) {
        payload_81();  // BMS PF status
    }
    // Priority 3: Regular power data (least frequent)
    else if (payload_01_time >= 180) {
        Power_UpdateData();
        payload_01();
        payload_01_time = 0;
    }
    else if (payload_11_time >= 180) {
        Power_UpdateData();
        payload_11();
        payload_11_time = 0;
    }
    else if (charger_flags[0] != charger_flags_prev[0] || charger_flags[1] != charger_flags_prev[1]) {
        payload_41();  // Only send on change (this is a status, not a fault)
        charger_flags_prev[0] = charger_flags[0];
        charger_flags_prev[1] = charger_flags[1];
    }
}








  


