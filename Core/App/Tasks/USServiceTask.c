#include "FreeRTOS.h"
#include "Types/SystemTypes.h"
#include "event_groups.h"
#include "main.h"
#include "max14808.h"
#include "sys_config.h"
#include "task.h"

void StartUltrasoundServiceTask(void *argument) {
  MAX14808_Reset();
  MAX14808_SetMode(MODE_OCTAL_THREE_LEVEL);
  MAX14808_SetCurrentLimit(CURRENT_LIMIT_2000mA);
  MAX14808_PulseGenerator_Init(&htim2, &htim5, &hdma_tim2_up);

  TickType_t xLastWakeTime = xTaskGetTickCount();
  EventBits_t sysEventbits;

  for (;;) {
    uint8_t scan_freq_hz = sysConfig.scan_frequency_hz;
    if (scan_freq_hz < CONFIG_SCAN_FREQ_MIN_HZ || scan_freq_hz > CONFIG_SCAN_FREQ_MAX_HZ) {
      scan_freq_hz = CONFIG_SCAN_FREQ_MIN_HZ;
    }

    // 向上取整，避免实际周期快于目标频率
    uint16_t period_ms = (uint16_t)((1000U + scan_freq_hz - 1U) / scan_freq_hz);
    TickType_t xFrequency = pdMS_TO_TICKS(period_ms);
    if (xFrequency == 0U) {
      xFrequency = 1U;
    }

    vTaskDelayUntil(&xLastWakeTime, xFrequency);

    if (sysEventGroupHandle != NULL) {
      sysEventbits = xEventGroupGetBits(sysEventGroupHandle);
      if (sysEventbits & SYS_EVENT_RUNNING_BIT) {
        taskENTER_CRITICAL();

        MAX14808_PulseGenerator_TriggerFromTask();

        taskEXIT_CRITICAL();
      } else {
        // SystemState 为 Stop 状态时，关闭所有通道，否则通道会持续处于 RECEIVE 状态
        // MAX14808_PulseGenerator_Stop();
        for (uint8_t i = 0; i < 8; i++) {
            MAX14808_SetChannel(i, OFF);
        }
      }
    }
  }
}