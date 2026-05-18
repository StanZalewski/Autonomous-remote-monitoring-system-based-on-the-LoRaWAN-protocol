#include "modbus_rtu.h"

// Pin definitions
#define DE_PIN  PA0
#define RE_PIN  PB5

// Calculate Modbus CRC-16
static uint16_t modbus_crc(uint8_t *data, uint8_t length) {
    uint16_t crc = 0xFFFF;
    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// Initialize Modbus RTU
void Modbus_Init() {
    pinMode(DE_PIN, OUTPUT);
    pinMode(RE_PIN, OUTPUT);
    
    digitalWrite(DE_PIN, LOW);
    digitalWrite(RE_PIN, LOW);
    
    Serial1.begin(115200);
}

// Read holding registers (Function Code 03)
uint8_t Modbus_ReadRegisters(uint8_t slave_addr, uint16_t start_reg, 
                              uint16_t reg_count, uint8_t *response, 
                              uint8_t maxLen) {
    uint8_t request[8];
    
    // Build request
    request[0] = slave_addr;
    request[1] = 0x03;  // FC03: Read Holding Registers
    request[2] = (start_reg >> 8) & 0xFF;
    request[3] = start_reg & 0xFF;
    request[4] = (reg_count >> 8) & 0xFF;
    request[5] = reg_count & 0xFF;
    
    // Add CRC
    uint16_t crc = modbus_crc(request, 6);
    request[6] = crc & 0xFF;
    request[7] = (crc >> 8) & 0xFF;
    
    // Clear RX buffer
    while(Serial1.available()) {
        Serial1.read();
    }
    
    // TX mode
    digitalWrite(DE_PIN, HIGH);
    digitalWrite(RE_PIN, HIGH);
    delay(10);
    
    // Send request
    Serial1.write(request, 8);
    Serial1.flush();
    
    // RX mode
    delay(10);
    digitalWrite(DE_PIN, LOW);
    digitalWrite(RE_PIN, LOW);
    
    // Wait for response
    uint32_t startTime = millis();
    uint8_t idx = 0;
    
    while(millis() - startTime < 2000 && idx < maxLen) {
        if(Serial1.available()) {
            response[idx++] = Serial1.read();
            startTime = millis();
        }
    }
    
    return idx;
}