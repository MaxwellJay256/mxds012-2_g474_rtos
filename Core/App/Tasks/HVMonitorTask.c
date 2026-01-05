/**
 * @file HVMonitorTask.c
 * @author Maxwell Jay (@MaxwellJay256)
 * @brief 高压电源监控任务，若检测到高压异常则关闭高压电源并通知系统停止
 * @version 0.1
 * @date 2026-01-04
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "FreeRTOS.h"
#include "task.h"
#include "Types/SystemTypes.h"
#include "cmsis_os2.h"
#include "lm51551.h"
#include "main.h"
#include <stdio.h>

void StartHVMonitorTask(void *argument) {
  for (;;) {
    /**
     * 读取 sysEventGroup 状态，如果没有运行标志则关闭高压电源
     * 如果有运行标志则开启高压电源
     * note: 这部分已经在 SysMonitorTask 中处理，这里保留备用
     */
    /*/
    if (sysEventGroupHandle != NULL) {
      EventBits_t bits = xEventGroupGetBits(sysEventGroupHandle);
      if (bits == 0) {
        HV_Disable();
      } else {
        HV_Enable();
      }
    }
    //*/
    // 处理高压异常情况
    if (!is_HV_PGood() &&
        HAL_GPIO_ReadPin(HV_EN_GPIO_Port, HV_EN_Pin) == GPIO_PIN_SET) {
      taskENTER_CRITICAL();
      HV_Disable();
      
      // 通知系统停止
      SysControlMessage* message = pvPortMalloc(sizeof(SysControlMessage));
      message->state = SystemState_Stop;
      osMessageQueuePut(sysControlQueueHandle, (&message), 0, osWaitForever);
      taskEXIT_CRITICAL();
      
      // 打印日志
      printf("[HVMonitorTask] High Voltage Fault Detected! Disabling HV Supply.\n");
    }

    osDelay(10);
  }
}