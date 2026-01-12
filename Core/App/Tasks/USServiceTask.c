#include "FreeRTOS.h"
#include "Types/SystemTypes.h"
#include "event_groups.h"
#include "main.h"
#include "max14808.h"
#include "task.h"

void StartUltrasoundServiceTask(void *argument) {
  MAX14808_Reset();
  MAX14808_SetMode(MODE_OCTAL_THREE_LEVEL);
  MAX14808_SetCurrentLimit(CURRENT_LIMIT_2000mA);
  MAX14808_PulseGenerator_Init(&htim2, &htim5, &hdma_tim2_up);

  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(50); // 50 ms周期
  EventBits_t sysEventbits;

  for (;;) {
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