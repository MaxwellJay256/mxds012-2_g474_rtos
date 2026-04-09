/**
 * @file           : ina231.c
 * @brief          : INA231 basic driver implementation.
 */

#include "ina231.h"

#define INA231_I2C_ADDRESS        (SLAVE_ADDRESS << 1)
#define INA231_I2C_TIMEOUT_MS     20U

/** INA231 寄存器地址 */
#define REG_CONFIG 0x00
#define REG_SHUNT_VOLTAGE 0x01
#define REG_BUS_VOLTAGE 0x02
#define REG_POWER 0x03
#define REG_CURRENT 0x04
#define REG_CALIBRATION 0x05
#define REG_MASK_ENABLE 0x06
#define REG_ALERT_LIMIT 0x07

#define INA231_CFG_RST_BIT        (1U << 15)
#define INA231_CFG_AVG_SHIFT      9U
#define INA231_CFG_AVG_MASK       (0x7U << INA231_CFG_AVG_SHIFT)
#define INA231_CFG_VBUSCT_SHIFT   6U
#define INA231_CFG_VBUSCT_MASK    (0x7U << INA231_CFG_VBUSCT_SHIFT)
#define INA231_CFG_VSHCT_SHIFT    3U
#define INA231_CFG_VSHCT_MASK     (0x7U << INA231_CFG_VSHCT_SHIFT)
#define INA231_CFG_MODE_MASK      0x7U

#define INA231_DEFAULT_CONFIG     0x0527U /* AVG=16, VBUSCT=1.1ms, VSHCT=1.1ms, MODE=continuous shunt+bus */
#define INA231_DEFAULT_CALIB      0x0280U /* Rshunt=20mOhm, Current_LSB=0.4mA/bit */

static HAL_StatusTypeDef INA231_WriteReg16(uint8_t reg, uint16_t value)
{
	uint8_t data[2];
	data[0] = (uint8_t)(value >> 8);
	data[1] = (uint8_t)(value & 0xFFU);

	return HAL_I2C_Mem_Write(&hi2c3,
							 INA231_I2C_ADDRESS,
							 reg,
							 I2C_MEMADD_SIZE_8BIT,
							 data,
							 2,
							 INA231_I2C_TIMEOUT_MS);
}

static HAL_StatusTypeDef INA231_ReadReg16(uint8_t reg, uint16_t *value)
{
	uint8_t data[2] = {0};
	HAL_StatusTypeDef status = HAL_I2C_Mem_Read(&hi2c3,
												 INA231_I2C_ADDRESS,
												 reg,
												 I2C_MEMADD_SIZE_8BIT,
												 data,
												 2,
												 INA231_I2C_TIMEOUT_MS);
	if (status != HAL_OK) {
		return status;
	}

	*value = (uint16_t)(((uint16_t)data[0] << 8) | data[1]);
	return HAL_OK;
}

void INA231_Write(uint16_t reg, uint32_t data)
{
	(void)INA231_WriteReg16((uint8_t)reg, (uint16_t)data);
}

uint32_t INA231_ReadRegister(uint16_t reg)
{
	uint16_t value = 0;
	if (INA231_ReadReg16((uint8_t)reg, &value) != HAL_OK) {
		return 0U;
	}

	return (uint32_t)value;
}

void INA231_Init(void)
{
	INA231_Reset();
	osDelay(2);
	INA231_Write(REG_CONFIG, INA231_DEFAULT_CONFIG);
	INA231_SetCalibration(INA231_DEFAULT_CALIB);
}

void INA231_Reset(void)
{
	INA231_Write(REG_CONFIG, INA231_CFG_RST_BIT);
}

void INA231_SetAveragingMode(uint8_t mode)
{
	uint16_t cfg = (uint16_t)INA231_ReadRegister(REG_CONFIG);
	cfg &= (uint16_t)(~INA231_CFG_AVG_MASK);
	cfg |= (uint16_t)(((uint16_t)mode & 0x7U) << INA231_CFG_AVG_SHIFT);
	INA231_Write(REG_CONFIG, cfg);
}

void INA231_SetVBUSConversionTime(uint8_t time)
{
	uint16_t cfg = (uint16_t)INA231_ReadRegister(REG_CONFIG);
	cfg &= (uint16_t)(~INA231_CFG_VBUSCT_MASK);
	cfg |= (uint16_t)(((uint16_t)time & 0x7U) << INA231_CFG_VBUSCT_SHIFT);
	INA231_Write(REG_CONFIG, cfg);
}

void INA231_SetVSHUNTConversionTime(uint8_t time)
{
	uint16_t cfg = (uint16_t)INA231_ReadRegister(REG_CONFIG);
	cfg &= (uint16_t)(~INA231_CFG_VSHCT_MASK);
	cfg |= (uint16_t)(((uint16_t)time & 0x7U) << INA231_CFG_VSHCT_SHIFT);
	INA231_Write(REG_CONFIG, cfg);
}

void INA231_SetOperatingMode(uint8_t mode)
{
	uint16_t cfg = (uint16_t)INA231_ReadRegister(REG_CONFIG);
	cfg &= (uint16_t)(~INA231_CFG_MODE_MASK);
	cfg |= (uint16_t)((uint16_t)mode & INA231_CFG_MODE_MASK);
	INA231_Write(REG_CONFIG, cfg);
}

void INA231_SetAlertMode(uint8_t mode)
{
	uint16_t mask = (uint16_t)INA231_ReadRegister(REG_MASK_ENABLE);
	mask &= 0xFFE0U;
	mask |= (uint16_t)(mode & 0x1FU);
	INA231_Write(REG_MASK_ENABLE, mask);
}

void INA231_SetCalibration(uint16_t value)
{
	INA231_Write(REG_CALIBRATION, value);
}

void INA231_SetAlertLimit(uint16_t limit)
{
	INA231_Write(REG_ALERT_LIMIT, limit);
}

int16_t INA231_ReadShuntVoltageRegister(void)
{
	return (int16_t)INA231_ReadRegister(REG_SHUNT_VOLTAGE);
}

uint16_t INA231_ReadBusVoltageRegister(void)
{
	return (uint16_t)INA231_ReadRegister(REG_BUS_VOLTAGE);
}

uint16_t INA231_ReadPowerRegister(void)
{
	return (uint16_t)INA231_ReadRegister(REG_POWER);
}

int16_t INA231_ReadCurrentRegister(void)
{
	return (int16_t)INA231_ReadRegister(REG_CURRENT);
}

float INA231_GetShuntVoltage_mV(void)
{
	return ((float)INA231_ReadShuntVoltageRegister()) * INA231_SHUNT_LSB_MV;
}

float INA231_GetBusVoltage_V(void)
{
	return ((float)INA231_ReadBusVoltageRegister()) * INA231_BUS_LSB_V;
}

float INA231_GetCurrent_mA(void)
{
	return ((float)INA231_ReadCurrentRegister()) * INA231_CURRENT_LSB_MA;
}

float INA231_GetPower_mW(void)
{
	return ((float)INA231_ReadPowerRegister()) * INA231_POWER_LSB_MW;
}
