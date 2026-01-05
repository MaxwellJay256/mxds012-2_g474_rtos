#include "cmsis_os2.h"
#include "main.h"

void StartADCTask(void * argument)
{
    for (;;) {
        // ADC 任务代码
        osDelay(10);
    }
}