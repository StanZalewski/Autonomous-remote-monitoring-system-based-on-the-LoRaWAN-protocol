#include "bq76952BMS.h"

HAL_StatusTypeDef BQ76952_ReadRegister(I2C_HandleTypeDef *hi2c, uint8_t reg_addr, uint8_t *data, uint8_t bytes)
{
    return HAL_I2C_Mem_Read(hi2c, BQ76952_I2C_ADDRESS << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, data, bytes, 100);
}

HAL_StatusTypeDef BQ76952_WriteRegister(I2C_HandleTypeDef *hi2c, uint8_t reg_addr, uint8_t *data, uint8_t bytes)
{
    return HAL_I2C_Mem_Write(hi2c, BQ76952_I2C_ADDRESS << 1, reg_addr, I2C_MEMADD_SIZE_8BIT, data, bytes, 100);
}

HAL_StatusTypeDef BQ76952_WriteRAM(I2C_HandleTypeDef *hi2c, uint16_t ram_addr, uint8_t *data, uint8_t length)
{
    HAL_StatusTypeDef status;
    uint8_t addr_low = ram_addr & 0xFF;
    uint8_t addr_high = (ram_addr >> 8) & 0xFF;
    
    // Write RAM address to 0x3E/0x3F (little-endian)
    status = BQ76952_WriteRegister(hi2c, 0x3E, &addr_low, 1);
    if (status != HAL_OK) return status;
    
    status = BQ76952_WriteRegister(hi2c, 0x3F, &addr_high, 1);
    if (status != HAL_OK) return status;
    
    // Write data to transfer buffer starting at 0x40
    status = BQ76952_WriteRegister(hi2c, 0x40, data, length);
    if (status != HAL_OK) return status;
    
    // Calculate checksum: ~(addr_high + addr_low + data_bytes)
    uint8_t checksum = 0;
    checksum += addr_low;
    checksum += addr_high;
    for (uint8_t i = 0; i < length; i++) {
        checksum += data[i];
    }
    checksum = ~checksum;
    
    // Write checksum and length (length includes 0x3E, 0x3F, 0x60, 0x61)
    uint8_t len_checksum[2];
    len_checksum[0] = checksum;
    len_checksum[1] = length + 4;
    
    status = BQ76952_WriteRegister(hi2c, 0x60, len_checksum, 2);
    HAL_Delay(5);
    
    return status;
}

HAL_StatusTypeDef BQ76952_ReadRAM(I2C_HandleTypeDef *hi2c, uint16_t ram_addr, uint8_t *data, uint8_t length)
{
    HAL_StatusTypeDef status;
    uint8_t addr_low = ram_addr & 0xFF;
    uint8_t addr_high = (ram_addr >> 8) & 0xFF;
    
    // Write RAM address to 0x3E/0x3F
    status = BQ76952_WriteRegister(hi2c, 0x3E, &addr_low, 1);
    if (status != HAL_OK) return status;
    
    status = BQ76952_WriteRegister(hi2c, 0x3F, &addr_high, 1);
    if (status != HAL_OK) return status;
    
    HAL_Delay(2);
    
    // Read data from transfer buffer
    return BQ76952_ReadRegister(hi2c, 0x40, data, length);
}

HAL_StatusTypeDef BQ76952_EnterConfigUpdateMode(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;
    uint8_t unseal_key_1[2] = {0x14, 0x04};
    uint8_t unseal_key_2[2] = {0x72, 0x36};
    uint8_t cfgupdate_cmd[2] = {0x90, 0x00};
    
    // Unseal the device with two-part key
    status = BQ76952_WriteRegister(hi2c, 0x3E, unseal_key_1, 2);
    if (status != HAL_OK) return status;
    HAL_Delay(5);
    
    status = BQ76952_WriteRegister(hi2c, 0x3E, unseal_key_2, 2);
    if (status != HAL_OK) return status;
    HAL_Delay(5);
    
    // Enter CONFIG_UPDATE mode
    status = BQ76952_WriteRegister(hi2c, 0x3E, cfgupdate_cmd, 2);
    if (status != HAL_OK) return status;
    HAL_Delay(10);
    
    // Verify CONFIG_UPDATE mode entered (bit 0 of Battery Status = 1)
    uint8_t battery_status;
    status = BQ76952_ReadRegister(hi2c, 0x12, &battery_status, 1);
    if (status != HAL_OK) return status;
    
    if ((battery_status & 0x01) == 0) {
        return HAL_ERROR;
    }
    
    return HAL_OK;
}

HAL_StatusTypeDef BQ76952_ExitConfigUpdate(I2C_HandleTypeDef *hi2c)
{
    uint8_t exit_cmd[2] = {0x92, 0x00};
    HAL_StatusTypeDef status = BQ76952_WriteRegister(hi2c, 0x3E, exit_cmd, 2);
    HAL_Delay(10);
    return status;
}

HAL_StatusTypeDef BQ76952_InitBasicConfig(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;
    uint8_t data[2];

    //Enable FET Control
    data[0] = 0x50;
    data[1] = 0x00;
    status = BQ76952_WriteRAM(hi2c, 0x9343, data, 2);
    if (status != HAL_OK) return status;
    
    // Min Blow Fuse Voltage: 32767 (max - disabled)
    data[0] = 0xFF; data[1] = 0x7F;
    status = BQ76952_WriteRAM(hi2c, 0x9231, data, 2);
    if (status != HAL_OK) return status;
    
    // Fuse Blow Timeout: 30 seconds
    data[0] = 30;
    status = BQ76952_WriteRAM(hi2c, 0x9233, data, 1);
    if (status != HAL_OK) return status;
    
    // Power Config: Sleep disabled, OTSD enabled, ADC 3ms, full-speed loops
    // Value: 0x2881
    data[0] = 0x81; data[1] = 0x28;
    status = BQ76952_WriteRAM(hi2c, 0x9234, data, 2);
    if (status != HAL_OK) return status;
    
    // REG12 Config: Both disabled
    data[0] = 0x00;
    status = BQ76952_WriteRAM(hi2c, 0x9236, data, 1);
    if (status != HAL_OK) return status;
    
    // REG0 Config: Disabled
    data[0] = 0x00;
    status = BQ76952_WriteRAM(hi2c, 0x9237, data, 1);
    if (status != HAL_OK) return status;
    
    // Vcell Mode: 3S config (Cell1, Cell2, Cell16)
    data[0] = 0x03; data[1] = 0x80;
    status = BQ76952_WriteRAM(hi2c, 0x9304, data, 2);
    if (status != HAL_OK) return status;
    
    // CC3 Samples: 70
    data[0] = 70;
    status = BQ76952_WriteRAM(hi2c, 0x9307, data, 1);
    if (status != HAL_OK) return status;
    
    return HAL_OK;
}

HAL_StatusTypeDef BQ76952_InitProtectionSettings(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;
    uint8_t data[1];
    
    // ✅ CORRECTED: Enabled Protections A
    // Bit layout: SCD OCD2 OCD1 OCC COV CUV RSVD RSVD
    // Enable all 6: 1111 1100 = 0xFC
    data[0] = 0xFC;
    status = BQ76952_WriteRAM(hi2c, 0x9261, data, 1);
    if (status != HAL_OK) return status;
    
    // ✅ CORRECTED: Enabled Protections B
    // Bit layout: OTF OTINT OTD OTC RSVD UTINT UTD UTC
    // Enable all 7: 1111 0111 = 0xF7
    data[0] = 0xF7;
    status = BQ76952_WriteRAM(hi2c, 0x9262, data, 1);
    if (status != HAL_OK) return status;
    
    // ✅ CORRECTED: Enabled Protections C
    // Bit layout: OCD3 SCDL OCDL COVL RSVD PTO HWDF RSVD
    // Enable OCD3 and HWDF: 1000 0010 = 0x82
    data[0] = 0x82;
    status = BQ76952_WriteRAM(hi2c, 0x9263, data, 1);
    if (status != HAL_OK) return status;
    
    // ✅ CORRECTED: CHG FET Protections A
    // Bit layout: SCD RSVD RSVD OCC COV RSVD RSVD RSVD
    // Enable SCD, OCC, COV: 1001 1000 = 0x98
    data[0] = 0x98;
    status = BQ76952_WriteRAM(hi2c, 0x9265, data, 1);
    if (status != HAL_OK) return status;
    
    // ✅ CORRECTED: CHG FET Protections B
    // Bit layout: OTF OTINT RSVD OTC RSVD UTINT RSVD UTC
    // Enable all: 1101 0101 = 0xD5
    data[0] = 0xD5;
    status = BQ76952_WriteRAM(hi2c, 0x9266, data, 1);
    if (status != HAL_OK) return status;
    
    // ✅ CORRECTED: CHG FET Protections C
    // Bit layout: RSVD SCDL RSVD COVL RSVD PTO HWDF RSVD
    // Enable SCDL, COVL, PTO, HWDF: 0101 0110 = 0x56
    data[0] = 0x56;
    status = BQ76952_WriteRAM(hi2c, 0x9267, data, 1);
    if (status != HAL_OK) return status;
    
    // ✅ CORRECTED: DSG FET Protections A
    // Bit layout: SCD OCD2 OCD1 RSVD RSVD CUV RSVD RSVD
    // Enable SCD, OCD2, OCD1, CUV: 1110 0100 = 0xE4
    data[0] = 0xE4;
    status = BQ76952_WriteRAM(hi2c, 0x9269, data, 1);
    if (status != HAL_OK) return status;
    
    // ✅ CORRECTED: DSG FET Protections B
    // Bit layout: OTF OTINT OTD RSVD RSVD UTINT UTD RSVD
    // Enable all: 1110 0110 = 0xE6
    data[0] = 0xE6;
    status = BQ76952_WriteRAM(hi2c, 0x926A, data, 1);
    if (status != HAL_OK) return status;
    
    // ✅ CORRECTED: DSG FET Protections C
    // Bit layout: OCD3 SCDL OCDL RSVD RSVD RSVD HWDF RSVD
    // Enable OCD3, SCDL, OCDL, HWDF: 1110 0010 = 0xE2
    data[0] = 0xE2;
    status = BQ76952_WriteRAM(hi2c, 0x926B, data, 1);
    if (status != HAL_OK) return status;
    
    return HAL_OK;
}

HAL_StatusTypeDef BQ76952_InitProtectionValues(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;
    uint8_t data[2];
    
    // ========== VOLTAGE PROTECTIONS ==========
    
    // CUV Threshold: 2500mV (safe cutoff for LiFePO4)
    data[0] = 49;  // 49 × 50.6mV = 2479mV
    status = BQ76952_WriteRAM(hi2c, 0x9275, data, 1);
    if (status != HAL_OK) return status;
    
    // CUV Delay: 500ms - CORRECTED from 1360ms!
    data[0] = 0x95; data[1] = 0x00;  // 149 → 497ms
    status = BQ76952_WriteRAM(hi2c, 0x9276, data, 2);
    if (status != HAL_OK) return status;
     
    // CUV Recovery Hysteresis: 200mV
    data[0] = 4;  // 4 × 50.6mV = 202mV
    status = BQ76952_WriteRAM(hi2c, 0x927B, data, 1);
    if (status != HAL_OK) return status;
    
    // COV Threshold: 3650mV (max charge voltage for LiFePO4)
    data[0] = 72;  // 72 × 50.6mV = 3643mV
    status = BQ76952_WriteRAM(hi2c, 0x9278, data, 1);
    if (status != HAL_OK) return status;
    
    // COV Delay: 200ms - CORRECTED from 1360ms!
    data[0] = 0x3A; data[1] = 0x00;  // 58 → 198ms
    status = BQ76952_WriteRAM(hi2c, 0x9279, data, 2);
    if (status != HAL_OK) return status;
    
    // COV Recovery Hysteresis: 100mV
    data[0] = 2;  // 2 × 50.6mV = 101mV
    status = BQ76952_WriteRAM(hi2c, 0x927C, data, 1);
    if (status != HAL_OK) return status;
    
    // ========== CHARGE CURRENT PROTECTION ==========
    
    // OCC Threshold: 4A @ 1mΩ (MINIMUM - cannot set to 1.5A due to hardware)
    // Your 1.5A charger limits current, this catches charger failures
    data[0] = 2;  // 2 × 2mV = 4mV = 4A @ 1mΩ
    status = BQ76952_WriteRAM(hi2c, 0x9280, data, 1);
    if (status != HAL_OK) return status;
    
    // OCC Delay: 200ms
    data[0] = 0x3A;  // 58 → 198ms
    status = BQ76952_WriteRAM(hi2c, 0x9281, data, 1);
    if (status != HAL_OK) return status;
    
    // OCC Recovery: -300mA
    data[0] = 0xD4; data[1] = 0xFE;  // -300mA
    status = BQ76952_WriteRAM(hi2c, 0x9288, data, 2);
    if (status != HAL_OK) return status;
    
    // ========== DISCHARGE CURRENT PROTECTIONS ==========
    
    // OCD1 Threshold: 6A @ 1mΩ (30% above 4.6A continuous)
    data[0] = 3;  // 3 × 2mV = 6mV = 6A @ 1mΩ
    status = BQ76952_WriteRAM(hi2c, 0x9282, data, 1);
    if (status != HAL_OK) return status;
    
    // OCD1 Delay: 300ms - CORRECTED (was impossible 1000ms!)
    data[0] = 0x59;  // 89 → 300ms (allows 10A/5s peaks)
    status = BQ76952_WriteRAM(hi2c, 0x9283, data, 1);
    if (status != HAL_OK) return status;
    
    // OCD2 Threshold: 12A @ 1mΩ (20% above 10A peak)
    data[0] = 6;  // 6 × 2mV = 12mV = 12A @ 1mΩ
    status = BQ76952_WriteRAM(hi2c, 0x9284, data, 1);
    if (status != HAL_OK) return status;
    
    // OCD2 Delay: 100ms
    data[0] = 0x1C;  // 28 → 99ms
    status = BQ76952_WriteRAM(hi2c, 0x9285, data, 1);
    if (status != HAL_OK) return status;
    
    // SCD Threshold: 60mV = 60A @ 1mΩ - CORRECTED from encoding error!
    data[0] = 0x03;  // 3 = 60mV (was also 3, which was actually correct)
    status = BQ76952_WriteRAM(hi2c, 0x9286, data, 1);
    if (status != HAL_OK) return status;
    
    // SCD Delay: 30µs - CORRECTED from 75µs!
    data[0] = 0x03;  // 3 → (3-1)×15 = 30µs
    status = BQ76952_WriteRAM(hi2c, 0x9287, data, 1);
    if (status != HAL_OK) return status;
    
    // SCD Recovery: 10 seconds
    data[0] = 10;
    status = BQ76952_WriteRAM(hi2c, 0x9294, data, 1);
    if (status != HAL_OK) return status;
    
    // OCD3 Threshold: -7A (optimized for 1.6Ah pack)
    data[0] = 0xA8; data[1] = 0xE4;  // -7000mA
    status = BQ76952_WriteRAM(hi2c, 0x928A, data, 2);
    if (status != HAL_OK) return status;
    
    // OCD3 Delay: 5 seconds
    data[0] = 5;
    status = BQ76952_WriteRAM(hi2c, 0x928C, data, 1);
    if (status != HAL_OK) return status;
    
    // OCD Recovery: +200mA
    data[0] = 0xC8; data[1] = 0x00;  // 200mA
    status = BQ76952_WriteRAM(hi2c, 0x928D, data, 2);
    if (status != HAL_OK) return status;
    
    // ========== TEMPERATURE PROTECTIONS ==========
    
    data[0] = 50; // OTC: 50°C
    status = BQ76952_WriteRAM(hi2c, 0x929A, data, 1);
    if (status != HAL_OK) return status;
    
    data[0] = 5; // OTC Delay: 5s
    status = BQ76952_WriteRAM(hi2c, 0x929B, data, 1);
    if (status != HAL_OK) return status;
    
    data[0] = 45; // OTC Recovery: 45°C
    status = BQ76952_WriteRAM(hi2c, 0x929C, data, 1);
    if (status != HAL_OK) return status;
    
    data[0] = 60; // OTD: 60°C
    status = BQ76952_WriteRAM(hi2c, 0x929D, data, 1);
    if (status != HAL_OK) return status;
    
    data[0] = 5; // OTD Delay: 5s
    status = BQ76952_WriteRAM(hi2c, 0x929E, data, 1);
    if (status != HAL_OK) return status;
    
    data[0] = 55; // OTD Recovery: 55°C
    status = BQ76952_WriteRAM(hi2c, 0x929F, data, 1);
    if (status != HAL_OK) return status;
    
    data[0] = 85; // OTINT: 85°C
    status = BQ76952_WriteRAM(hi2c, 0x92A3, data, 1);
    if (status != HAL_OK) return status;
    
    data[0] = 2; // OTINT Delay: 2s
    status = BQ76952_WriteRAM(hi2c, 0x92A4, data, 1);
    if (status != HAL_OK) return status;
    
    data[0] = 80; // OTINT Recovery: 80°C
    status = BQ76952_WriteRAM(hi2c, 0x92A5, data, 1);
    if (status != HAL_OK) return status;
    
    data[0] = 0; // UTC: 0°C
    status = BQ76952_WriteRAM(hi2c, 0x92A6, data, 1);
    if (status != HAL_OK) return status;
    
    data[0] = 5; // UTC Delay: 5s
    status = BQ76952_WriteRAM(hi2c, 0x92A7, data, 1);
    if (status != HAL_OK) return status;
    
    data[0] = 5; // UTC Recovery: 5°C
    status = BQ76952_WriteRAM(hi2c, 0x92A8, data, 1);
    if (status != HAL_OK) return status;
    
    data[0] = (uint8_t)(-20); // UTD: -20°C
    status = BQ76952_WriteRAM(hi2c, 0x92A9, data, 1);
    if (status != HAL_OK) return status;
    
    data[0] = 5; // UTD Delay: 5s
    status = BQ76952_WriteRAM(hi2c, 0x92AA, data, 1);
    if (status != HAL_OK) return status;
    
    data[0] = (uint8_t)(-15); // UTD Recovery: -15°C
    status = BQ76952_WriteRAM(hi2c, 0x92AB, data, 1);
    if (status != HAL_OK) return status;
    
    data[0] = (uint8_t)(-40); // UTINT: -40°C
    status = BQ76952_WriteRAM(hi2c, 0x92AC, data, 1);
    if (status != HAL_OK) return status;
    
    data[0] = 5; // UTINT Delay: 5s
    status = BQ76952_WriteRAM(hi2c, 0x92AD, data, 1);
    if (status != HAL_OK) return status;
    
    data[0] = (uint8_t)(-35); // UTINT Recovery: -35°C
    status = BQ76952_WriteRAM(hi2c, 0x92AE, data, 1);
    if (status != HAL_OK) return status;
    
    // ========== GENERAL SETTINGS ==========
    
    // General recovery time: 1 second
    data[0] = 1;
    status = BQ76952_WriteRAM(hi2c, 0x92AF, data, 1);
    if (status != HAL_OK) return status;
    
    // Host Watchdog Delay: 80 seconds
    data[0] = 80; data[1] = 0x00;
    status = BQ76952_WriteRAM(hi2c, 0x92B2, data, 2);
    if (status != HAL_OK) return status;
    
    // Body Diode Threshold: 100mA
    data[0] = 0x64; data[1] = 0x00;
    status = BQ76952_WriteRAM(hi2c, 0x9273, data, 2);
    if (status != HAL_OK) return status;
    
    return HAL_OK;
}

HAL_StatusTypeDef BQ76952_InitFETAndPins(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;
    uint8_t data[1];
    
    // FET Options: Series mode, CHG OFF in sleep, host control, FETs on at init
    data[0] = 0x2D;
    status = BQ76952_WriteRAM(hi2c, 0x9308, data, 1);
    if (status != HAL_OK) return status;
    
    // Charge Pump: Enabled, 11V overdrive, source-follower in SLEEP
    data[0] = 0x05;
    status = BQ76952_WriteRAM(hi2c, 0x9309, data, 1);
    if (status != HAL_OK) return status;
    
    // ALERT Pin: Active-low, tri-state (external pull-up)
    // LOW when alarm active, tri-state HIGH when inactive
    data[0] = 0x82;
    status = BQ76952_WriteRAM(hi2c, 0x92FC, data, 1);
    if (status != HAL_OK) return status;
    
    // DCHG Pin: Active-low polarity (HIGH when CHG FET ON, LOW when disabled)
    data[0] = 0x8A;
    status = BQ76952_WriteRAM(hi2c, 0x9301, data, 1);
    if (status != HAL_OK) return status;
    
    // DDSG Pin: Active-low polarity (HIGH when DSG FET ON, LOW when disabled)
    data[0] = 0x8A;
    status = BQ76952_WriteRAM(hi2c, 0x9302, data, 1);
    if (status != HAL_OK) return status;
    
    // CFETOFF: Disabled
    data[0] = 0x00;
    status = BQ76952_WriteRAM(hi2c, 0x92FA, data, 1);
    if (status != HAL_OK) return status;
    
    // DFETOFF: Disabled
    data[0] = 0x00;
    status = BQ76952_WriteRAM(hi2c, 0x92FB, data, 1);
    if (status != HAL_OK) return status;
    
    // TS1: 18kΩ model, cell temp, enabled
    data[0] = 0x07;
    status = BQ76952_WriteRAM(hi2c, 0x92FD, data, 1);
    if (status != HAL_OK) return status;
    
    // TS2: Disabled
    data[0] = 0x00;
    status = BQ76952_WriteRAM(hi2c, 0x92FE, data, 1);
    if (status != HAL_OK) return status;
    
    // TS3: Disabled
    data[0] = 0x00;
    status = BQ76952_WriteRAM(hi2c, 0x92FF, data, 1);
    if (status != HAL_OK) return status;
    
    return HAL_OK;
}

HAL_StatusTypeDef BQ76952_InitCellBalancing(I2C_HandleTypeDef *hi2c)
{
    HAL_StatusTypeDef status;
    uint8_t data[2];
    
    // ===== 0x9335: Balancing Configuration =====
    // Bit 1 (CB_RLX) = 1: Balance during relax
    // Bit 0 (CB_CHG) = 0: Don't balance during charge
    data[0] = 0x02;
    status = BQ76952_WriteRAM(hi2c, 0x9335, data, 1);
    if (status != HAL_OK) return status;

    // ===== 0x933A: Cell Balance Max Cells =====
    // Balance 1 cell at a time (default, keep it)
    // For 3S pack, could increase to 3 for faster balancing
    data[0] = 1;  // 1 cell at a time (conservative)
    status = BQ76952_WriteRAM(hi2c, 0x933A, data, 1);
    if (status != HAL_OK) return status;
    
    // ===== 0x933F: Cell Balance Min Cell V (Relax) =====
    // Don't balance below 3.0V during relax
    // Default is 3900mV, lower to 3000mV for LiFePO4
    data[0] = 0xB8; data[1] = 0x0B;  // 3000mV (I2 type, little-endian)
    status = BQ76952_WriteRAM(hi2c, 0x933F, data, 2);
    if (status != HAL_OK) return status;
    
    // ===== 0x9341: Cell Balance Min Delta (Relax) =====
    // Start balancing when cells differ by >40mV
    // Default: 40mV (keep it - good for LiFePO4)
    data[0] = 40;
    status = BQ76952_WriteRAM(hi2c, 0x9341, data, 1);
    if (status != HAL_OK) return status;
    
    // ===== 0x9342: Cell Balance Stop Delta (Relax) =====
    // Stop when cells within 20mV
    // Default: 20mV (keep it)
    data[0] = 20;
    status = BQ76952_WriteRAM(hi2c, 0x9342, data, 1);
    if (status != HAL_OK) return status;
    
    return HAL_OK;
}

// Quick verification of critical registers only
// Returns HAL_OK if all critical values correct, HAL_ERROR if any mismatch
HAL_StatusTypeDef BQ76952_VerifyCritical(I2C_HandleTypeDef *hi2c)
{
    uint8_t val;
    
    // Critical Power Config - 0x9234 low byte
    BQ76952_ReadRAM(hi2c, 0x9234, &val, 1);
    if (val != 0x81) return HAL_ERROR;
    
    // FET Options - must be 0x2D (FETs off initially)
    BQ76952_ReadRAM(hi2c, 0x9308, &val, 1);
    if (val != 0x2D) return HAL_ERROR;
    
    // ALERT Pin - must be 0x82
    BQ76952_ReadRAM(hi2c, 0x92FC, &val, 1);
    if (val != 0x82) return HAL_ERROR;
    
    // DCHG Pin - must be 0x8A
    BQ76952_ReadRAM(hi2c, 0x9301, &val, 1);
    if (val != 0x8A) return HAL_ERROR;
    
    // DDSG Pin - must be 0x8A
    BQ76952_ReadRAM(hi2c, 0x9302, &val, 1);
    if (val != 0x8A) return HAL_ERROR;
    
    // Protection Enables A - must be 0xFC
    BQ76952_ReadRAM(hi2c, 0x9261, &val, 1);
    if (val != 0xFC) return HAL_ERROR;
    
    // CUV Threshold - must be 49 (2.5V)
    BQ76952_ReadRAM(hi2c, 0x9275, &val, 1);
    if (val != 49) return HAL_ERROR;
    
    // COV Threshold - must be 72 (3.65V)
    BQ76952_ReadRAM(hi2c, 0x9278, &val, 1);
    if (val != 72) return HAL_ERROR;
    
    // OCC Threshold - must be 2 (4A)
    BQ76952_ReadRAM(hi2c, 0x9280, &val, 1);
    if (val != 2) return HAL_ERROR;
    
    // OCD1 Threshold - must be 3 (6A)
    BQ76952_ReadRAM(hi2c, 0x9282, &val, 1);
    if (val != 3) return HAL_ERROR;
    
    // SCD Threshold - must be 0x03 (60mV/60A)
    BQ76952_ReadRAM(hi2c, 0x9286, &val, 1);
    if (val != 0x03) return HAL_ERROR;
    
    // Cell Balancing Config - must be 0x02
    BQ76952_ReadRAM(hi2c, 0x9335, &val, 1);
    if (val != 0x02) return HAL_ERROR;
    
    return HAL_OK; // All critical registers verified!
}