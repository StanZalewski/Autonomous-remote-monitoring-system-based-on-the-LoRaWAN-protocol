#include "rs485_dock.h"
#include "modbus_rtu.h"

// Slave addresses
#define WEATHER_STATION_ADDR  2
#define POWER_BOARD_ADDR      1

// ========== GLOBAL VARIABLES - WEATHER STATION ==========
uint16_t temp_SHT45 = 0;
uint16_t humidity_SHT45 = 0;
uint32_t lux_VEML7700 = 0;
uint16_t whiteRatio_VEML7700 = 0;
uint16_t temp_BMP390 = 0;
uint32_t pressure_BMP390 = 0;

// ========== GLOBAL VARIABLES - POWER BOARD ==========
uint16_t cell1_voltage = 0;
uint16_t cell2_voltage = 0;
uint16_t cell3_voltage = 0;
uint16_t current_pack = 0;
uint16_t pack_temp_BMS = 0;
uint16_t pack_temp_Charger = 0;
uint16_t vac1_voltage = 0;
uint16_t gen_sys_status = 0;
uint16_t fault_flags[2] = {0, 0};
uint16_t charger_flags[2] = {0, 0};
uint16_t bms_safety[3] = {0, 0, 0};
uint16_t bms_pf_alert[2] = {0, 0};
uint16_t bms_pf_status[2] = {0, 0};

// Update weather station data
void Weather_UpdateData() {
    uint8_t buffer[64];
    uint8_t len;
    
    // Read 8 registers from weather station
    len = Modbus_ReadRegisters(WEATHER_STATION_ADDR, 0, 8, buffer, sizeof(buffer));
    
    if(len == 21) {  // Expected: 1 (addr) + 1 (FC) + 1 (count) + 16 (data) + 2 (CRC)
        // Parse response (skip addr, FC, count - start at index 3)
        // Data is Big-Endian (high byte first)
        
        temp_SHT45 = (buffer[3] << 8) | buffer[4];           // Reg 0
        humidity_SHT45 = (buffer[5] << 8) | buffer[6];       // Reg 1
        lux_VEML7700 = ((uint32_t)buffer[7] << 24) |        // Reg 2-3
                       ((uint32_t)buffer[8] << 16) |
                       ((uint32_t)buffer[9] << 8) |
                       buffer[10];
        whiteRatio_VEML7700 = (buffer[11] << 8) | buffer[12]; // Reg 4
        temp_BMP390 = (buffer[13] << 8) | buffer[14];       // Reg 5
        pressure_BMP390 = ((uint32_t)buffer[15] << 24) |    // Reg 6-7
                          ((uint32_t)buffer[16] << 16) |
                          ((uint32_t)buffer[17] << 8) |
                          buffer[18];
        
        // DEBUG: Print unscaled values
        Serial.println("\n=== WEATHER STATION DATA ===");
        Serial.print("temp_SHT45:         0x");
        if(temp_SHT45 < 0x1000) Serial.print("0");
        if(temp_SHT45 < 0x100) Serial.print("0");
        if(temp_SHT45 < 0x10) Serial.print("0");
        Serial.print(temp_SHT45, HEX);
        Serial.print(" (");
        Serial.print(temp_SHT45);
        Serial.println(")");
        
        Serial.print("humidity_SHT45:     0x");
        if(humidity_SHT45 < 0x1000) Serial.print("0");
        if(humidity_SHT45 < 0x100) Serial.print("0");
        if(humidity_SHT45 < 0x10) Serial.print("0");
        Serial.print(humidity_SHT45, HEX);
        Serial.print(" (");
        Serial.print(humidity_SHT45);
        Serial.println(")");
        
        Serial.print("lux_VEML7700:       0x");
        Serial.print(lux_VEML7700, HEX);
        Serial.print(" (");
        Serial.print(lux_VEML7700);
        Serial.println(")");
        
        Serial.print("whiteRatio_VEML7700:0x");
        if(whiteRatio_VEML7700 < 0x1000) Serial.print("0");
        if(whiteRatio_VEML7700 < 0x100) Serial.print("0");
        if(whiteRatio_VEML7700 < 0x10) Serial.print("0");
        Serial.print(whiteRatio_VEML7700, HEX);
        Serial.print(" (");
        Serial.print(whiteRatio_VEML7700);
        Serial.println(")");
        
        Serial.print("temp_BMP390:        0x");
        if(temp_BMP390 < 0x1000) Serial.print("0");
        if(temp_BMP390 < 0x100) Serial.print("0");
        if(temp_BMP390 < 0x10) Serial.print("0");
        Serial.print(temp_BMP390, HEX);
        Serial.print(" (");
        Serial.print(temp_BMP390);
        Serial.println(")");
        
        Serial.print("pressure_BMP390:    0x");
        Serial.print(pressure_BMP390, HEX);
        Serial.print(" (");
        Serial.print(pressure_BMP390);
        Serial.println(")");
        Serial.println("============================\n");
        
    } else {
        Serial.print("WEATHER STATION ERROR: Expected 21 bytes, got ");
        Serial.println(len);
    }
}

// Update power board regular data (Registers 0-7)
void Power_UpdateData() {
    uint8_t buffer[64];
    uint8_t len;
    
    // Read 8 registers (regular data only)
    len = Modbus_ReadRegisters(POWER_BOARD_ADDR, 0, 8, buffer, sizeof(buffer));
    
    if(len == 21) {  // Expected: 1 + 1 + 1 + 16 (data) + 2 (CRC)
        // Parse response (skip addr, FC, count - start at index 3)
        
        cell1_voltage = (buffer[3] << 8) | buffer[4];       // Reg 0
        cell2_voltage = (buffer[5] << 8) | buffer[6];       // Reg 1
        cell3_voltage = (buffer[7] << 8) | buffer[8];       // Reg 2
        current_pack = (buffer[9] << 8) | buffer[10];       // Reg 3
        pack_temp_BMS = (buffer[11] << 8) | buffer[12];     // Reg 4
        pack_temp_Charger = (buffer[13] << 8) | buffer[14]; // Reg 5
        vac1_voltage = (buffer[15] << 8) | buffer[16];      // Reg 6
        gen_sys_status = (buffer[17] << 8) | buffer[18];    // Reg 7
        
        // DEBUG: Print values
        Serial.println("\n=== POWER BOARD DATA ===");
        Serial.print("cell1_voltage:      0x");
        if(cell1_voltage < 0x1000) Serial.print("0");
        if(cell1_voltage < 0x100) Serial.print("0");
        if(cell1_voltage < 0x10) Serial.print("0");
        Serial.print(cell1_voltage, HEX);
        Serial.print(" (");
        Serial.print(cell1_voltage);
        Serial.println(" mV)");
        
        Serial.print("cell2_voltage:      0x");
        if(cell2_voltage < 0x1000) Serial.print("0");
        if(cell2_voltage < 0x100) Serial.print("0");
        if(cell2_voltage < 0x10) Serial.print("0");
        Serial.print(cell2_voltage, HEX);
        Serial.print(" (");
        Serial.print(cell2_voltage);
        Serial.println(" mV)");
        
        Serial.print("cell3_voltage:      0x");
        if(cell3_voltage < 0x1000) Serial.print("0");
        if(cell3_voltage < 0x100) Serial.print("0");
        if(cell3_voltage < 0x10) Serial.print("0");
        Serial.print(cell3_voltage, HEX);
        Serial.print(" (");
        Serial.print(cell3_voltage);
        Serial.println(" mV)");
        
        Serial.print("current_pack:       0x");
        if(current_pack < 0x1000) Serial.print("0");
        if(current_pack < 0x100) Serial.print("0");
        if(current_pack < 0x10) Serial.print("0");
        Serial.print(current_pack, HEX);
        Serial.print(" (");
        Serial.print((int16_t)current_pack);
        Serial.println(" mA)");
        
        Serial.print("vac1_voltage:       0x");
        if(vac1_voltage < 0x1000) Serial.print("0");
        if(vac1_voltage < 0x100) Serial.print("0");
        if(vac1_voltage < 0x10) Serial.print("0");
        Serial.print(vac1_voltage, HEX);
        Serial.print(" (");
        Serial.print(vac1_voltage);
        Serial.println(" mV)");
        
        Serial.print("gen_sys_status:     0x");
        if(gen_sys_status < 0x1000) Serial.print("0");
        if(gen_sys_status < 0x100) Serial.print("0");
        if(gen_sys_status < 0x10) Serial.print("0");
        Serial.print(gen_sys_status, HEX);
        Serial.print(" (");
        Serial.print(gen_sys_status);
        Serial.println(")");
        
        Serial.println("========================\n");
        
    } else {
        Serial.print("POWER BOARD DATA ERROR: Expected 21 bytes, got ");
        Serial.println(len);
    }
}

// Update power board fault/flag data (Registers 8-18)
void Power_UpdateFaults() {
    uint8_t buffer[64];
    uint8_t len;
    
    // Read 11 registers (fault/flag data only)
    len = Modbus_ReadRegisters(POWER_BOARD_ADDR, 8, 11, buffer, sizeof(buffer));
    
    if(len == 27) {  // Expected: 1 + 1 + 1 + 22 (data) + 2 (CRC)
        // Parse response (skip addr, FC, count - start at index 3)
        
        fault_flags[0] = (buffer[3] << 8) | buffer[4];      // Reg 8
        fault_flags[1] = (buffer[5] << 8) | buffer[6];      // Reg 9
        charger_flags[0] = (buffer[7] << 8) | buffer[8];    // Reg 10
        charger_flags[1] = (buffer[9] << 8) | buffer[10];   // Reg 11
        
        bms_safety[0] = (buffer[11] << 8) | buffer[12];     // Reg 12
        bms_safety[1] = (buffer[13] << 8) | buffer[14];     // Reg 13
        bms_safety[2] = (buffer[15] << 8) | buffer[16];     // Reg 14
        
        bms_pf_alert[0] = (buffer[17] << 8) | buffer[18];   // Reg 15
        bms_pf_alert[1] = (buffer[19] << 8) | buffer[20];   // Reg 16
        
        bms_pf_status[0] = (buffer[21] << 8) | buffer[22];  // Reg 17
        bms_pf_status[1] = (buffer[23] << 8) | buffer[24];  // Reg 18
        
        // DEBUG: Print flags
        Serial.println("\n=== POWER BOARD FAULTS ===");
        Serial.print("fault_flags:    0x");
        Serial.print(fault_flags[0], HEX);
        Serial.print(" 0x");
        Serial.println(fault_flags[1], HEX);
        
        Serial.print("charger_flags:  0x");
        Serial.print(charger_flags[0], HEX);
        Serial.print(" 0x");
        Serial.println(charger_flags[1], HEX);
        
        Serial.print("bms_safety:     0x");
        Serial.print(bms_safety[0], HEX);
        Serial.print(" 0x");
        Serial.print(bms_safety[1], HEX);
        Serial.print(" 0x");
        Serial.println(bms_safety[2], HEX);
        
        Serial.print("bms_pf_alert:   0x");
        Serial.print(bms_pf_alert[0], HEX);
        Serial.print(" 0x");
        Serial.println(bms_pf_alert[1], HEX);
        
        Serial.print("bms_pf_status:  0x");
        Serial.print(bms_pf_status[0], HEX);
        Serial.print(" 0x");
        Serial.println(bms_pf_status[1], HEX);
        
        Serial.println("==========================\n");
        
    } else {
        Serial.print("POWER BOARD FAULTS ERROR: Expected 27 bytes, got ");
        Serial.println(len);
    }
}