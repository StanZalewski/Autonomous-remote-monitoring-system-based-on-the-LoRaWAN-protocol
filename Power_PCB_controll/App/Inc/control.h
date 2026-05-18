#ifndef CONTROL_H
#define CONTROL_H

#include "stm32g0xx_hal.h"
extern I2C_HandleTypeDef hi2c1;

typedef struct {
    // Cell voltages (mV)
    int16_t cell1_voltage;
    int16_t cell2_voltage;
    int16_t cell3_voltage;
    int16_t current_pack;
    uint8_t gen_sys_status;
    int16_t pack_temperature_BMS;
    uint16_t pack_temperature_Charger;
    uint16_t vac1_voltage;
    
} SystemData;

extern SystemData g_system_data;


HAL_StatusTypeDef GenSysInfo(void);
void WatchDogs_R (void);

#endif