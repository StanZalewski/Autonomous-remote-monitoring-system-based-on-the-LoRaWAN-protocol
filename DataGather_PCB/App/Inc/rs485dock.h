#ifndef RS485DOCK_H
#define RS485DOCK_H

#include "stm32g0xx_hal.h"
#include <stdint.h>

void RS485_Init(UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim);
void RS485_UpdateRegisters(void);
void RS485_Process(void);

#endif