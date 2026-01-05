#include "FreeRTOS.h"
#include "Types/SystemTypes.h"
#include "event_groups.h"
#include "main.h"
#include "max14808.h"
#include "task.h"


void StartUltrasoundServiceTask(void *argument) {
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
      }
    }
  }
}