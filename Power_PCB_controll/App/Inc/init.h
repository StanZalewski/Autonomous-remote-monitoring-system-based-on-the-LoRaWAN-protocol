#ifndef INIT_H
#define INIT_H

#include "stm32g0xx_hal.h"
#include <stdint.h>
extern I2C_HandleTypeDef hi2c1;

typedef struct {
    uint8_t fault_check_0;
    uint8_t fault_check_1;
    uint8_t fault_flag_0;
    uint8_t fault_flag_1;
} FaultCheckflags;

typedef struct {
    uint8_t charger_status_0;
    uint8_t charger_status_1;
    uint8_t charger_status_2;
    uint8_t charger_status_3;
    uint8_t charger_status_4;
    uint8_t charger_flag_0;
    uint8_t charger_flag_1;
    uint8_t charger_flag_2;
    uint8_t charger_flag_3;
} ChargerStatusflags;

typedef struct {
    uint8_t safety_alert_a;
    uint8_t safety_alert_b;
    uint8_t safety_alert_c;
    uint8_t safety_status_a;
    uint8_t safety_status_b;
    uint8_t safety_status_c;
    uint8_t pf_alert_a;
    uint8_t pf_alert_b;
    uint8_t pf_alert_c;
    uint8_t pf_alert_d;
    uint8_t pf_status_a;
    uint8_t pf_status_b;
    uint8_t pf_status_c;
    uint8_t pf_status_d;
    uint8_t battery_status;
} BMSStatusFlags;

extern FaultCheckflags g_fault_check;
extern ChargerStatusflags g_charger_status;
extern BMSStatusFlags g_bms_status;

HAL_StatusTypeDef BQ25798_HealthCheck();
HAL_StatusTypeDef BQ76952_HealthCheck(I2C_HandleTypeDef *hi2c);

void BQ25798_Init();
HAL_StatusTypeDef BQ76952_Init(void);
HAL_StatusTypeDef fullInitialization(void);

#endif