#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "main.h"
#include "led.h"
#include "Types/SystemTypes.h"
#include "event_groups.h"

void StartLEDTask(void * argument)
{
    for (;;) {
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
        osDelay(20);
    }
}