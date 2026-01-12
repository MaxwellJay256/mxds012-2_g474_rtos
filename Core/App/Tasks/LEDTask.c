#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "Types/SystemTypes.h"
#include "event_groups.h"
#include "led.h"
#include "main.h"

#define LEDTASK_POLL_INTERVAL_MS 20 // LED 任务轮询间隔

void StartLEDTask(void *argument) {
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(LEDTASK_POLL_INTERVAL_MS);
  
  for (;;) {
    vTaskDelayUntil(&xLastWakeTime, xFrequency);
    // LED 任务代码
    if (sysEventGroupHandle != NULL) {
      EventBits_t sysEventbits = xEventGroupGetBits(sysEventGroupHandle);
      if (sysEventbits & SYS_EVENT_RUNNING_BIT) {
        LED_ResetColor(RED);
        LED_EnableColor(GREEN);
      } else {
        LED_SetColor(RED);
      }
    }
  }
}