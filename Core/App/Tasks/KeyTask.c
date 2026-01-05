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

void StartKeyTask(void * argument)
{
    SystemState nextState = SystemState_Stop; // 首次按下进入 Running
    for (;;) {
        if (isKeyClicked()) {
            // 切换目标状态
            nextState = (nextState == SystemState_Running) ? SystemState_Stop : SystemState_Running;

            SysControlMessage* message = pvPortMalloc(sizeof(SysControlMessage));
            message->state = nextState;
            osMessageQueuePut(sysControlQueueHandle, (&message), 0, osWaitForever);
        }

        osDelay(10);
    }
}