/**
 * @file KeyTask.c
 * @author Maxwell Jay (@MaxwellJay256)
 * @brief 
 * @version 0.1
 * @date 2026-01-02
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "cmsis_os2.h"
#include "main.h"
#include "FreeRTOS.h"
#include "key.h"
#include "Types/SystemTypes.h"
#include "event_groups.h"

#define KEYTASK_POLL_INTERVAL_MS 10 // 按键轮询间隔

void StartKeyTask(void * argument)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(KEYTASK_POLL_INTERVAL_MS);
    
    SystemState nextState = SystemState_Stop; // 首次按下进入 Running
    EventBits_t sysEventbits;
    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
        if (isKeyClicked()) {
            // 切换目标状态
            sysEventbits = xEventGroupGetBits(sysEventGroupHandle);
            nextState = (sysEventbits & SYS_EVENT_RUNNING_BIT) ? SystemState_Stop : SystemState_Running;

            SysControlMessage* message = pvPortMalloc(sizeof(SysControlMessage));
            message->state = nextState;
            osMessageQueuePut(sysControlQueueHandle, (&message), 0, osWaitForever);
        }
    }
}