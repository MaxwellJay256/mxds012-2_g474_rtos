/**
 * @file           : ina231.h
 * @brief          : Header for ina231.c file.
 */

#ifndef INA231_H
#define INA231_H

#include "i2c.h"
#include <stdint.h>

#define SLAVE_ADDRESS 0b1000000 // INA231 地址

/** INA231 寄存器地址 */
#define REG_CONFIG 0x00
#define REG_SHUNT_VOLTAGE 0x01
#define REG_BUS_VOLTAGE 0x02
#define REG_POWER 0x03
#define REG_CURRENT 0x04
#define REG_CALIBRATION 0x05
#define REG_MASK_ENABLE 0x06
#define REG_ALERT_LIMIT 0x07

void INA231_Write(uint16_t reg, uint32_t data);
uint32_t INA231_ReadRegister(uint16_t reg);

/** INA231 Configurations */
typedef enum {
    AVERAGE_NUM_1 = 0b000,
    AVERAGE_NUM_4,
    AVERAGE_NUM_16 = 0b010,
    AVERAGE_NUM_64,
    AVERAGE_NUM_128 = 0b100,
    AVERAGE_NUM_256,
    AVERAGE_NUM_512,
    AVERAGE_NUM_1024 = 0b111
} AVG_BITS;


void INA231_Init(void);
void INA231_Reset(void);
void INA231_SetAveragingMode(uint8_t mode);
void INA231_SetVBUSConversionTime(uint8_t time);
void INA231_SetVSHUNTConversionTime(uint8_t time);
void INA231_SetOperatingMode(uint8_t mode);
void INA231_SetAlertMode(uint8_t mode);

/** INA231 Calibration */
void INA231_SetCalibration(uint16_t value);
void INA231_SetAlertLimit(uint16_t limit);

/** INA231 Measurements */
int16_t INA231_ReadShuntVoltageRegister(void);
uint16_t INA231_ReadBusVoltageRegister(void);
uint16_t INA231_ReadPowerRegister(void);
int16_t INA231_ReadCurrentRegister(void);
float INA231_GetShuntVoltage_mV(void);
float INA231_GetBusVoltage_V(void);
float INA231_GetCurrent_mA(void);
float INA231_GetPower_mW(void);

#endif /* INA231_H */