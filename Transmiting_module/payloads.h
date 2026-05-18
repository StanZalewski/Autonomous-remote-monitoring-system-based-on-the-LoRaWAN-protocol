#ifndef PAYLOADS_H
#define PAYLOADS_H

#include <Arduino.h>

#define TIMING_PAYLOAD_01 180
#define TIMING_PAYLOAD_11 180
#define TIMING_PAYLOAD_02 5
#define TIMING_PAYLOAD_12 10

extern uint8_t payload_01_wait;
extern uint8_t payload_11_wait;
extern uint8_t payload_02_wait;
extern uint8_t payload_12_wait;
extern uint8_t payload_22_wait;

// Function declarations
void payload_01();
void payload_11();
void payload_02();
void payload_12();
void payload_21();
void payload_31();
void payload_41();
void payload_51();
void payload_61();
void payload_71();
void payload_81();

#endif