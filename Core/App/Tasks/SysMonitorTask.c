#include "FreeRTOS.h"
#include "Types/SystemTypes.h"
#include "cmsis_os2.h"
#include "led.h"
#include "lm51551.h"
#include "usb_device.h"
#include "main.h"
#include <stdio.h>


/**
 * @brief 系统管理任务，处理系统状态切换
 *
 * @param argument
 */
void StartSysMonitorTask(void *argument) {
  /* init code for USB_Device */
  MX_USB_Device_Init();

  SystemState currentState = SystemState_Stop;
  for (;;) {
    SysControlMessage *message = NULL;
    if (osMessageQueueGet(sysControlQueueHandle, &message, 0, osWaitForever) != osOK) {
      continue;
    }
    if (message == NULL) {
      continue;
    }

    if (message->state == currentState) {
      vPortFree(message);
      continue;
    }

    currentState = message->state;
    if (currentState == SystemState_Running) {
      HV_Enable();
      osEventFlagsSet(sysEventGroupHandle, SYS_EVENT_RUNNING_BIT);
      printf("[SysMonitorTask] System Running. High Voltage Enabled.\n");
      // LED_SetColor(GREEN);
    } else {
      osEventFlagsClear(sysEventGroupHandle, SYS_EVENT_RUNNING_BIT);
      // HV_Disable();
      // printf("[SysMonitorTask] System Stopped. High Voltage Disabled.\n");
      // LED_SetColor(RED);
    }
    vPortFree(message);
  }
}