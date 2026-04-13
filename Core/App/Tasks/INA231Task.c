/**
 * @file INA231Task.c
 * @author Maxwell Jay (@MaxwellJay256)
 * @brief 
 * @version 0.2
 * @date 2026-04-10
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "cmsis_os2.h"
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ina231.h"
#include "Types/UsbComTypes.h"

#define INA231_TASK_PERIOD_MS 2000

static uint8_t INA231_USB_TransmitPower(uint32_t bus_mv, int32_t current_ma, uint32_t power_mw)
{
	const uint16_t payload_len = 6U;
	UsbFrame *frame;
	uint8_t *payload;
	UsbFrame *to_send;

	if (UsbTxQueueHandle == NULL) {
		return 1U;
	}

	if (bus_mv > 0xFFFFU) {
		bus_mv = 0xFFFFU;
	}

	if (current_ma > INT16_MAX) {
		current_ma = INT16_MAX;
	} else if (current_ma < INT16_MIN) {
		current_ma = INT16_MIN;
	}

	if (power_mw > 0xFFFFU) {
		power_mw = 0xFFFFU;
	}

	frame = (UsbFrame *)pvPortMalloc(sizeof(UsbFrame));
	if (frame == NULL) {
		return 1U;
	}

	payload = (uint8_t *)pvPortMalloc(payload_len);
	if (payload == NULL) {
		vPortFree(frame);
		return 1U;
	}

	payload[0] = (uint8_t)(bus_mv & 0xFFU); // 总线电压低 8 位
	payload[1] = (uint8_t)((bus_mv >> 8) & 0xFFU); // 总线电压高 8 位
	payload[2] = (uint8_t)((uint16_t)current_ma & 0xFFU); // 电流低 8 位
	payload[3] = (uint8_t)(((uint16_t)current_ma >> 8) & 0xFFU); // 电流高 8 位
	payload[4] = (uint8_t)(power_mw & 0xFFU); // 功率低 8 位
	payload[5] = (uint8_t)((power_mw >> 8) & 0xFFU); // 功率高 8 位

	frame->type = USB_PKT_TYPE_POWER;
	frame->payload_len = payload_len;
	frame->payload = payload;

	to_send = frame;
	if (osMessageQueuePut(UsbTxQueueHandle, &to_send, 0, 0) != osOK) {
		vPortFree(payload);
		vPortFree(frame);
		return 1U;
	}

	return 0U;
}

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
		const int16_t raw_current = INA231_ReadCurrentRegister();
		const uint16_t raw_power = INA231_ReadPowerRegister();

		/* Bus: 1.25mV/bit, Current: 1mA/bit, Power: 10mW/bit */
		const uint32_t bus_mv = ((uint32_t)raw_bus * 125U) / 100U;
		const int32_t current_ma = (int32_t)(raw_current * 4) / 10;
		const uint32_t power_mw = (uint32_t)raw_power * 10U;

		INA231_USB_TransmitPower(bus_mv, current_ma, power_mw);
	}
}