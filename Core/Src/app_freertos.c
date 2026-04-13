/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Types/SystemTypes.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for SysMonitorTask */
osThreadId_t SysMonitorTaskHandle;
const osThreadAttr_t SysMonitorTask_attributes = {
  .name = "SysMonitorTask",
  .priority = (osPriority_t) osPriorityNormal7,
  .stack_size = 128 * 4
};
/* Definitions for OLEDTask */
osThreadId_t OLEDTaskHandle;
const osThreadAttr_t OLEDTask_attributes = {
  .name = "OLEDTask",
  .priority = (osPriority_t) osPriorityNormal3,
  .stack_size = 128 * 4
};
/* Definitions for LEDTask */
osThreadId_t LEDTaskHandle;
const osThreadAttr_t LEDTask_attributes = {
  .name = "LEDTask",
  .priority = (osPriority_t) osPriorityBelowNormal,
  .stack_size = 128 * 4
};
/* Definitions for USServiceTask */
osThreadId_t USServiceTaskHandle;
const osThreadAttr_t USServiceTask_attributes = {
  .name = "USServiceTask",
  .priority = (osPriority_t) osPriorityRealtime,
  .stack_size = 128 * 4
};
/* Definitions for KeyTask */
osThreadId_t KeyTaskHandle;
const osThreadAttr_t KeyTask_attributes = {
  .name = "KeyTask",
  .priority = (osPriority_t) osPriorityAboveNormal,
  .stack_size = 128 * 4
};
/* Definitions for HVMonitorTask */
osThreadId_t HVMonitorTaskHandle;
const osThreadAttr_t HVMonitorTask_attributes = {
  .name = "HVMonitorTask",
  .priority = (osPriority_t) osPriorityAboveNormal1,
  .stack_size = 128 * 4
};
/* Definitions for INA231Task */
osThreadId_t INA231TaskHandle;
const osThreadAttr_t INA231Task_attributes = {
  .name = "INA231Task",
  .priority = (osPriority_t) osPriorityLow7,
  .stack_size = 128 * 4
};
/* Definitions for ADCTask */
osThreadId_t ADCTaskHandle;
const osThreadAttr_t ADCTask_attributes = {
  .name = "ADCTask",
  .priority = (osPriority_t) osPriorityRealtime,
  .stack_size = 128 * 4
};
/* Definitions for USBRXTask */
osThreadId_t USBRXTaskHandle;
const osThreadAttr_t USBRXTask_attributes = {
  .name = "USBRXTask",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 128 * 4
};
/* Definitions for USBTXTask */
osThreadId_t USBTXTaskHandle;
const osThreadAttr_t USBTXTask_attributes = {
  .name = "USBTXTask",
  .priority = (osPriority_t) osPriorityNormal6,
  .stack_size = 128 * 4
};
/* Definitions for sysControlQueue */
osMessageQueueId_t sysControlQueueHandle;
const osMessageQueueAttr_t sysControlQueue_attributes = {
  .name = "sysControlQueue"
};
/* Definitions for OLEDQueue */
osMessageQueueId_t OLEDQueueHandle;
const osMessageQueueAttr_t OLEDQueue_attributes = {
  .name = "OLEDQueue"
};
/* Definitions for USBRXQueue */
osMessageQueueId_t USBRXQueueHandle;
const osMessageQueueAttr_t USBRXQueue_attributes = {
  .name = "USBRXQueue"
};
/* Definitions for UsbTxQueue */
osMessageQueueId_t UsbTxQueueHandle;
const osMessageQueueAttr_t UsbTxQueue_attributes = {
  .name = "UsbTxQueue"
};
/* Definitions for usbTxMutex */
osMutexId_t usbTxMutexHandle;
const osMutexAttr_t usbTxMutex_attributes = {
  .name = "usbTxMutex"
};
/* Definitions for USBTXCpltSem */
osSemaphoreId_t USBTXCpltSemHandle;
const osSemaphoreAttr_t USBTXCpltSem_attributes = {
  .name = "USBTXCpltSem"
};
/* Definitions for sysEventGroup */
osEventFlagsId_t sysEventGroupHandle;
const osEventFlagsAttr_t sysEventGroup_attributes = {
  .name = "sysEventGroup"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartSysMonitorTask(void *argument);
void StartOLEDTask(void *argument);
extern void StartLEDTask(void *argument);
extern void StartUltrasoundServiceTask(void *argument);
extern void StartKeyTask(void *argument);
extern void StartHVMonitorTask(void *argument);
void StartINA231Task(void *argument);
extern void StartADCTask(void *argument);
void StartUSBRXTask(void *argument);
void StartUSBTXTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of usbTxMutex */
  usbTxMutexHandle = osMutexNew(&usbTxMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* creation of USBTXCpltSem */
  USBTXCpltSemHandle = osSemaphoreNew(1, 0, &USBTXCpltSem_attributes);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of sysControlQueue */
  sysControlQueueHandle = osMessageQueueNew (8, sizeof(SysControlMessage*), &sysControlQueue_attributes);

  /* creation of OLEDQueue */
  OLEDQueueHandle = osMessageQueueNew (16, sizeof(uint16_t), &OLEDQueue_attributes);

  /* creation of USBRXQueue */
  USBRXQueueHandle = osMessageQueueNew (16, sizeof(uint8_t), &USBRXQueue_attributes);

  /* creation of UsbTxQueue */
  UsbTxQueueHandle = osMessageQueueNew (16, sizeof(void*), &UsbTxQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of SysMonitorTask */
  SysMonitorTaskHandle = osThreadNew(StartSysMonitorTask, NULL, &SysMonitorTask_attributes);

  /* creation of OLEDTask */
  OLEDTaskHandle = osThreadNew(StartOLEDTask, NULL, &OLEDTask_attributes);

  /* creation of LEDTask */
  LEDTaskHandle = osThreadNew(StartLEDTask, NULL, &LEDTask_attributes);

  /* creation of USServiceTask */
  USServiceTaskHandle = osThreadNew(StartUltrasoundServiceTask, NULL, &USServiceTask_attributes);

  /* creation of KeyTask */
  KeyTaskHandle = osThreadNew(StartKeyTask, NULL, &KeyTask_attributes);

  /* creation of HVMonitorTask */
  HVMonitorTaskHandle = osThreadNew(StartHVMonitorTask, NULL, &HVMonitorTask_attributes);

  /* creation of INA231Task */
  INA231TaskHandle = osThreadNew(StartINA231Task, NULL, &INA231Task_attributes);

  /* creation of ADCTask */
  ADCTaskHandle = osThreadNew(StartADCTask, NULL, &ADCTask_attributes);

  /* creation of USBRXTask */
  USBRXTaskHandle = osThreadNew(StartUSBRXTask, NULL, &USBRXTask_attributes);

  /* creation of USBTXTask */
  USBTXTaskHandle = osThreadNew(StartUSBTXTask, NULL, &USBTXTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* creation of sysEventGroup */
  sysEventGroupHandle = osEventFlagsNew(&sysEventGroup_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartSysMonitorTask */
/**
  * @brief  Function implementing the sysMonitorTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartSysMonitorTask */
__weak void StartSysMonitorTask(void *argument)
{
  /* init code for USB_Device */
  MX_USB_Device_Init();
  /* USER CODE BEGIN StartSysMonitorTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartSysMonitorTask */
}

/* USER CODE BEGIN Header_StartOLEDTask */
/**
* @brief Function implementing the OLEDTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartOLEDTask */
__weak void StartOLEDTask(void *argument)
{
  /* USER CODE BEGIN StartOLEDTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartOLEDTask */
}

/* USER CODE BEGIN Header_StartINA231Task */
/**
* @brief Function implementing the INA231Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartINA231Task */
__weak void StartINA231Task(void *argument)
{
  /* USER CODE BEGIN StartINA231Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartINA231Task */
}

/* USER CODE BEGIN Header_StartUSBRXTask */
/**
* @brief Function implementing the USBRXTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUSBRXTask */
__weak void StartUSBRXTask(void *argument)
{
  /* USER CODE BEGIN StartUSBRXTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartUSBRXTask */
}

/* USER CODE BEGIN Header_StartUSBTXTask */
/**
* @brief Function implementing the USBTXTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUSBTXTask */
__weak void StartUSBTXTask(void *argument)
{
  /* USER CODE BEGIN StartUSBTXTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartUSBTXTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

