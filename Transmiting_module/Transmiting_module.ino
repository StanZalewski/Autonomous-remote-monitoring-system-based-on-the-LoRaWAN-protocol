#include "loraWAN_dock.h"
#include "modbus_rtu.h"
#include "rs485_dock.h"
#include "payloads.h"


#define PIN_LED_DISABLE     PA9

void setup() {
    Serial.begin(115200);
    delay(2000);

    pinMode(PIN_LED_DISABLE, OUTPUT);
    digitalWrite(PIN_LED_DISABLE, LOW);


    Serial.println("Starting RAK3172...");
    
    // Enable low power mode
    api.system.lpm.set(1);
    
    // Initialize Modbus RS485
    Modbus_Init();
    Serial.println("Modbus initialized");
    
    // Initialize LoRaWAN
    loraInit();
    Serial.println("LoRaWAN joined");
    
    // Create and start timer
    if (api.system.timer.create(RAK_TIMER_0, sendLoRaCallback, RAK_TIMER_PERIODIC)) {
        Serial.println("Timer created");
        if (api.system.timer.start(RAK_TIMER_0, 60000, NULL)) {
            Serial.println("Timer started - every 60s");
        }
    }
    
    Serial.println("Setup complete!");
}

void loop() {
    api.system.sleep.all(0);  // Sleep until timer wakes device
}