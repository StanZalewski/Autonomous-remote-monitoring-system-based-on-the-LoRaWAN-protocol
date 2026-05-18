#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include "stm32g0xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

// Configuration
#define MODBUS_SLAVE_ADDRESS    1
#define MODBUS_MAX_FRAME_SIZE   256
#define MODBUS_REGISTER_COUNT   19  // Increased for control register

// Modbus Function Codes
#define MODBUS_FC_READ_HOLDING_REGISTERS    0x03
#define MODBUS_FC_WRITE_SINGLE_REGISTER     0x06

// Modbus Exception Codes
#define MODBUS_EXCEPTION_ILLEGAL_FUNCTION       0x01
#define MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS   0x02
#define MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE     0x03

// Control Register Commands
#define CONTROL_REG_ADDRESS     12
#define CMD_RESET_DEVICE        0xAA55

// Modbus States
typedef enum {
    MODBUS_STATE_IDLE,
    MODBUS_STATE_RECEIVING,
    MODBUS_STATE_PROCESSING,
    MODBUS_STATE_TRANSMITTING
} Modbus_State_t;

// Modbus RTU Structure
typedef struct {
    uint8_t rxBuffer[MODBUS_MAX_FRAME_SIZE];
    uint8_t txBuffer[MODBUS_MAX_FRAME_SIZE];
    uint16_t rxIndex;
    uint16_t txLength;
    Modbus_State_t state;
    uint16_t registers[MODBUS_REGISTER_COUNT];
} Modbus_RTU_t;

// Public Functions
void Modbus_Init(UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim);
void Modbus_Process(void);
void Modbus_UART_RxCallback(void);
void Modbus_TIM_Callback(void);
void Modbus_UART_TxCompleteCallback(void);
void Modbus_SetRegisterU32(uint16_t startReg, uint32_t value);
void Modbus_SetRegisterU16(uint16_t reg, uint16_t value); 
uint32_t Modbus_GetRegisterU32(uint16_t startReg);

#endif /* MODBUS_RTU_H */