#ifndef ADC_SYNC_H
#define ADC_SYNC_H

#include "stm32g4xx_hal.h"
#include "adc.h"
#include "tim.h"
#include "dma.h"
#include "usb_tx.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>

#define ADC_RESULT_BUFFER_LENGTH 30

// 初始化：预置 ADC 多模 + DMA，准备好等待 TRGO 触发
void ADC_Sync_Init(void);

void ADC_Sync_Start(void);
void MAX14808_PulseGenerator_DMATransferComplete(DMA_HandleTypeDef *hdma);

void ADC_Sync_TransmitResultsIfFull(void);
bool USB_ADC_Results_Transmit5(const int16_t *const results[5], uint16_t length);
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

// USB 输出数据包格式：AA | type | lenL | lenH | payload | 0D 0A
#define USB_PKT_HEADER      0xAA
#define USB_PKT_TYPE_ADC    0x01
#define USB_PKT_TAIL0       0x0D
#define USB_PKT_TAIL1       0x0A

// ADC 数据帧（打包后交给 USBTX 任务发送）
typedef struct {
	uint16_t len;   // 完整包长度（含头尾）
	uint8_t *buf;   // 动态分配的包缓冲
} ADCUsbPacket;

#endif // ADC_SYNC_H