#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include <Arduino.h>

// Initialize Modbus RTU communication
void Modbus_Init();

// Low-level Modbus transaction
// Returns: Number of bytes received (0 if error)
uint8_t Modbus_ReadRegisters(uint8_t slave_addr, uint16_t start_reg, 
                              uint16_t reg_count, uint8_t *response, 
                              uint8_t maxLen);

#endif