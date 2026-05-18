/**
 * Modbus RTU Slave Driver for STM32G030F6
 * Production version with fixed UART interrupt handling
 */

#include "modbus_rtu.h"
#include "main.h"

// Private variables
static Modbus_RTU_t modbus;
static UART_HandleTypeDef *pHuart;
static TIM_HandleTypeDef *pHtim;
static volatile bool resetRequested = false;

// CRITICAL: Fixed single-byte buffer for UART RX
static uint8_t uart_rx_byte;

// RS485 Control
#define RS485_TX_MODE() do { \
    HAL_GPIO_WritePin(DE_RS485_GPIO_Port, DE_RS485_Pin, GPIO_PIN_SET); \
    HAL_GPIO_WritePin(RE_RS485_GPIO_Port, RE_RS485_Pin, GPIO_PIN_SET); \
} while(0)

#define RS485_RX_MODE() do { \
    HAL_GPIO_WritePin(DE_RS485_GPIO_Port, DE_RS485_Pin, GPIO_PIN_RESET); \
    HAL_GPIO_WritePin(RE_RS485_GPIO_Port, RE_RS485_Pin, GPIO_PIN_RESET); \
} while(0)

// Private functions
static uint16_t Modbus_CRC16(uint8_t *buffer, uint16_t length);
static void Modbus_SendResponse(uint8_t *data, uint16_t length);
static void Modbus_SendException(uint8_t functionCode, uint8_t exceptionCode);
static void Modbus_ProcessFrame(void);
static void Modbus_HandleReadHoldingRegisters(uint8_t *frame, uint16_t length);
static void Modbus_HandleWriteSingleRegister(uint8_t *frame, uint16_t length);
static void Modbus_ProcessControlCommand(uint16_t command);

void Modbus_Init(UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim)
{
    pHuart = huart;
    pHtim = htim;
    
    modbus.rxIndex = 0;
    modbus.txLength = 0;
    modbus.state = MODBUS_STATE_IDLE;
    resetRequested = false;
    
    // Clear registers
    for(uint16_t i = 0; i < MODBUS_REGISTER_COUNT; i++) {
        modbus.registers[i] = 0;
    }
    
    // Set RS485 to receive mode
    RS485_RX_MODE();
    
    // Enable UART RX interrupt - always use fixed buffer location
    HAL_UART_Receive_IT(pHuart, &uart_rx_byte, 1);
}

void Modbus_Process(void)
{
    if(modbus.state == MODBUS_STATE_PROCESSING) {
        Modbus_ProcessFrame();
        modbus.rxIndex = 0;
        modbus.state = MODBUS_STATE_IDLE;
    }
    
    // Check if reset was requested
    if(resetRequested) {
        resetRequested = false;
        HAL_NVIC_SystemReset();
    }
}

void Modbus_UART_RxCallback(void)
{
    // Copy byte from fixed buffer into frame buffer
    if(modbus.rxIndex < MODBUS_MAX_FRAME_SIZE) {
        modbus.rxBuffer[modbus.rxIndex] = uart_rx_byte;
        modbus.rxIndex++;
    }
    
    // Restart 3.5 character timer
    __HAL_TIM_SET_COUNTER(pHtim, 0);
    HAL_TIM_Base_Start_IT(pHtim);
    
    modbus.state = MODBUS_STATE_RECEIVING;
    
    // Prevent buffer overflow
    if(modbus.rxIndex >= MODBUS_MAX_FRAME_SIZE) {
        modbus.rxIndex = 0;
        modbus.state = MODBUS_STATE_IDLE;
        HAL_TIM_Base_Stop_IT(pHtim);
    }
    
    // Continue receiving - always into same fixed buffer location
    HAL_UART_Receive_IT(pHuart, &uart_rx_byte, 1);
}

void Modbus_TIM_Callback(void)
{
    HAL_TIM_Base_Stop_IT(pHtim);
    
    if(modbus.state == MODBUS_STATE_RECEIVING && modbus.rxIndex >= 4) {
        modbus.state = MODBUS_STATE_PROCESSING;
    } else {
        modbus.state = MODBUS_STATE_IDLE;
        modbus.rxIndex = 0;
    }
}

void Modbus_UART_TxCompleteCallback(void)
{
    // Not used with blocking transmission
}

static void Modbus_ProcessFrame(void)
{
    uint8_t *frame = modbus.rxBuffer;
    uint16_t length = modbus.rxIndex;
    
    // Validate minimum frame length
    if(length < 4) {
        return;
    }
    
    // Check CRC (Modbus RTU: LOW byte first, HIGH byte second)
    uint16_t receivedCRC = frame[length-2] | (frame[length-1] << 8);
    uint16_t calculatedCRC = Modbus_CRC16(frame, length - 2);
    
    if(receivedCRC != calculatedCRC) {
        return; // Invalid CRC
    }
    
    // Check slave address
    if(frame[0] != MODBUS_SLAVE_ADDRESS) {
        return; // Not for us
    }
    
    // Process function code
    uint8_t functionCode = frame[1];
    
    switch(functionCode) {
        case MODBUS_FC_READ_HOLDING_REGISTERS:
            Modbus_HandleReadHoldingRegisters(frame, length);
            break;
            
        case MODBUS_FC_WRITE_SINGLE_REGISTER:
            Modbus_HandleWriteSingleRegister(frame, length);
            break;
            
        default:
            Modbus_SendException(functionCode, MODBUS_EXCEPTION_ILLEGAL_FUNCTION);
            break;
    }
}

static void Modbus_HandleReadHoldingRegisters(uint8_t *frame, uint16_t length)
{
    if(length != 8) {
        Modbus_SendException(MODBUS_FC_READ_HOLDING_REGISTERS, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
        return;
    }
    
    uint16_t startAddr = (frame[2] << 8) | frame[3];
    uint16_t regCount = (frame[4] << 8) | frame[5];
    
    if(regCount == 0 || regCount > 125) {
        Modbus_SendException(MODBUS_FC_READ_HOLDING_REGISTERS, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
        return;
    }
    
    if(startAddr + regCount > MODBUS_REGISTER_COUNT) {
        Modbus_SendException(MODBUS_FC_READ_HOLDING_REGISTERS, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }
    
    // Small delay for timing
    HAL_Delay(10);
    
    // Build response
    uint8_t byteCount = regCount * 2;
    modbus.txBuffer[0] = MODBUS_SLAVE_ADDRESS;
    modbus.txBuffer[1] = MODBUS_FC_READ_HOLDING_REGISTERS;
    modbus.txBuffer[2] = byteCount;
    
    // Copy register values (Big-Endian)
    for(uint16_t i = 0; i < regCount; i++) {
        uint16_t regValue = modbus.registers[startAddr + i];
        modbus.txBuffer[3 + i*2] = (regValue >> 8) & 0xFF;
        modbus.txBuffer[3 + i*2 + 1] = regValue & 0xFF;
    }
    
    Modbus_SendResponse(modbus.txBuffer, 3 + byteCount);
}

static void Modbus_HandleWriteSingleRegister(uint8_t *frame, uint16_t length)
{
    if(length != 8) {
        Modbus_SendException(MODBUS_FC_WRITE_SINGLE_REGISTER, MODBUS_EXCEPTION_ILLEGAL_DATA_VALUE);
        return;
    }
    
    uint16_t regAddress = (frame[2] << 8) | frame[3];
    uint16_t regValue = (frame[4] << 8) | frame[5];
    
    if(regAddress >= MODBUS_REGISTER_COUNT) {
        Modbus_SendException(MODBUS_FC_WRITE_SINGLE_REGISTER, MODBUS_EXCEPTION_ILLEGAL_DATA_ADDRESS);
        return;
    }
    
    // Write to register
    modbus.registers[regAddress] = regValue;
    
    // Check if it's a control command
    if(regAddress == CONTROL_REG_ADDRESS) {
        Modbus_ProcessControlCommand(regValue);
    }
    
    // Echo back the request as response (standard FC06 behavior)
    modbus.txBuffer[0] = MODBUS_SLAVE_ADDRESS;
    modbus.txBuffer[1] = MODBUS_FC_WRITE_SINGLE_REGISTER;
    modbus.txBuffer[2] = frame[2];
    modbus.txBuffer[3] = frame[3];
    modbus.txBuffer[4] = frame[4];
    modbus.txBuffer[5] = frame[5];
    
    Modbus_SendResponse(modbus.txBuffer, 6);
}

static void Modbus_ProcessControlCommand(uint16_t command)
{
    switch(command) {
        case CMD_RESET_DEVICE:
            resetRequested = true;
            break;
            
        // Add more control commands here as needed
        default:
            break;
    }
}

static void Modbus_SendResponse(uint8_t *data, uint16_t length)
{
    uint16_t crc = Modbus_CRC16(data, length);
    data[length] = crc & 0xFF;
    data[length + 1] = (crc >> 8) & 0xFF;
    
    modbus.txLength = length + 2;
    modbus.state = MODBUS_STATE_TRANSMITTING;
    
    // Small delay for master to switch to RX mode
    HAL_Delay(20);
    
    RS485_TX_MODE();
    HAL_Delay(1);

    // Enable UART transmitter
    SET_BIT(pHuart->Instance->CR1, USART_CR1_TE);
    HAL_Delay(1);  // Wait for TX to stabilize

    HAL_UART_Transmit(pHuart, modbus.txBuffer, modbus.txLength, 100);

    // Wait for transmission to fully complete
    while(!(pHuart->Instance->ISR & USART_ISR_TC));

    HAL_Delay(5);

    // Disable UART transmitter to save power
    CLEAR_BIT(pHuart->Instance->CR1, USART_CR1_TE);

    RS485_RX_MODE();
    
    modbus.rxIndex = 0;
    modbus.state = MODBUS_STATE_IDLE;
    
    // Re-enable RX interrupt into fixed buffer
    HAL_UART_Receive_IT(pHuart, &uart_rx_byte, 1);
}

static void Modbus_SendException(uint8_t functionCode, uint8_t exceptionCode)
{
    modbus.txBuffer[0] = MODBUS_SLAVE_ADDRESS;
    modbus.txBuffer[1] = functionCode | 0x80;
    modbus.txBuffer[2] = exceptionCode;
    
    Modbus_SendResponse(modbus.txBuffer, 3);
}

static uint16_t Modbus_CRC16(uint8_t *buffer, uint16_t length)
{
    uint16_t crc = 0xFFFF;
    
    for(uint16_t i = 0; i < length; i++) {
        crc ^= (uint16_t)buffer[i];
        
        for(uint8_t j = 0; j < 8; j++) {
            if(crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return crc;
}

void Modbus_SetRegisterU32(uint16_t startReg, uint32_t value)
{
    if(startReg + 1 < MODBUS_REGISTER_COUNT) {
        modbus.registers[startReg] = (value >> 16) & 0xFFFF;
        modbus.registers[startReg + 1] = value & 0xFFFF;
    }
}

void Modbus_SetRegisterU16(uint16_t reg, uint16_t value)
{
    if(reg < MODBUS_REGISTER_COUNT) {
        modbus.registers[reg] = value;
    }
}

uint32_t Modbus_GetRegisterU32(uint16_t startReg)
{
    if(startReg + 1 < MODBUS_REGISTER_COUNT) {
        uint32_t highWord = modbus.registers[startReg];
        uint32_t lowWord = modbus.registers[startReg + 1];
        return (highWord << 16) | lowWord;
    }
    return 0;
}