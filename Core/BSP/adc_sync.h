#ifndef ADC_SYNC_H
#define ADC_SYNC_H

#include "FreeRTOS.h"
#include "adc.h"
#include "dma.h"
#include "stm32g4xx_hal.h"
#include "task.h"
#include "tim.h"
#include "usb_com.h"
#include "sys_config.h"
#include <stdint.h>


#define ADC_RESULT_BUFFER_LENGTH CONFIG_SAMPLE_DEPTH_MAX // ADC 通道缓冲容量上限

extern volatile uint8_t adc_data_ready; // ADC 数据就绪标志，供任务检查

// 初始化：预置 ADC 多模 + DMA，准备好等待 TRGO 触发
void ADC_Sync_Init(void);

void ADC_Sync_Start(void);
void MAX14808_PulseGenerator_DMATransferComplete(DMA_HandleTypeDef *hdma);

void ADC_Sync_TransmitResultsIfFull(void);
uint8_t USB_ADC_Transmit_Bin(const int16_t *const results[4], uint16_t length);
// 初始化微秒计时（启用 DWT CYCCNT），在系统初始化后调用一次
void ADC_Timestamp_Init(void);

// Register a FreeRTOS task handle which will be notified from ISR when a
// full ADC result set is ready. The registered task should block on a
// notification (e.g. ulTaskNotifyTake) and call
// ADC_Sync_TransmitResultsIfFull() to handle data and restart DMA.
void ADC_Sync_SetNotifyTask(TaskHandle_t task);

// Start ADC sampling immediately (e.g., called after pulse DMA completes).
// This function is typically called from a task context.
void ADC_Sync_StartSamplingTimer(void);

#endif // ADC_SYNC_H