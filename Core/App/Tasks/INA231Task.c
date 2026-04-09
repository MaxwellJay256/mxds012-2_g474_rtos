/**
 * @file INA231Task.c
 * @author Maxwell Jay (@MaxwellJay256)
 * @brief 
 * @version 0.1
 * @date 2026-03-23
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "cmsis_os2.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ina231.h"
#include "Types/SystemTypes.h"
#include <stdio.h>
#include "led.h"

#define INA231_TASK_PERIOD_MS 1000

void StartINA231Task(void *argument)
{
	(void)argument;

	INA231_Init();
	osDelay(10);

  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(INA231_TASK_PERIOD_MS);

	for (;;) {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);

		const uint16_t raw_bus = INA231_ReadBusVoltageRegister();
		const int16_t raw_shunt = INA231_ReadShuntVoltageRegister();
		const int16_t raw_current = INA231_ReadCurrentRegister();
		const uint16_t raw_power = INA231_ReadPowerRegister();

		/* Bus: 1.25mV/bit, Shunt: 2.5uV/bit, Current: 1mA/bit, Power: 25mW/bit */
		const uint32_t bus_mv = ((uint32_t)raw_bus * 125U) / 100U;
		const int32_t shunt_uv = ((int32_t)raw_shunt * 25) / 10;
		const int32_t shunt_uv_abs = (shunt_uv < 0) ? -shunt_uv : shunt_uv;
		const int32_t current_ma = (int32_t)(raw_current * 4) / 10;
		const uint32_t power_mw = (uint32_t)raw_power * 10U;

		printf("[INA231] Vbus=%lu.%03lu V, Vshunt=%c%ld.%03ld mV, I=%ld mA, P=%lu mW\n",
					 (unsigned long)(bus_mv / 1000U),
					 (unsigned long)(bus_mv % 1000U),
					 (shunt_uv < 0) ? '-' : '+',
					 (long)(shunt_uv_abs / 1000),
					 (long)(shunt_uv_abs % 1000),
					 (long)current_ma,
					 (unsigned long)power_mw);
    // const float bus_v = INA231_GetBusVoltage_V();
		// const float shunt_mv = INA231_GetShuntVoltage_mV();
		// const float current_ma = INA231_GetCurrent_mA();
		// const float power_mw = INA231_GetPower_mW();
    // printf("[INA231] Vbus=%.3f V, Vshunt=%.3f mV, I=%.3f mA, P=%.3f mW\n",
		// 			 bus_v,
		// 			 shunt_mv,
		// 			 current_ma,pyinstaller --noconfirm --clean --windowed --onedir --name MXDS012-Host main.py --collect-all qfluentwidgets --collect-all pyqtgraph --collect-submodules PyQt5 --hidden-import serial.tools.list_ports
		// 			 power_mw);
	}
}