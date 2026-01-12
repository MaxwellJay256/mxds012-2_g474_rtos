/**
 * @file adc_sync.c
 * @author Maxwell Jay (@MaxwellJay256)
 * @brief 
 * @version 0.1
 * @date 2026-01-11
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "adc_sync.h"
#include "adc.h"
#include "core_cm4.h"
#include "stm32g4xx_hal_adc.h"
#include "stm32g4xx_hal_tim.h"
#include "tim.h"
#include "usb_tx.h"
#include "cmsis_os.h"
#include "main.h"
#include <stdio.h>
#include <string.h>


static int16_t adc_result[4][ADC_RESULT_BUFFER_LENGTH];
// 多模打包缓冲：ADC1+ADC2、ADC3+ADC4 各用 32-bit 打包缓冲
static uint32_t adc12_packed[ADC_RESULT_BUFFER_LENGTH];
static uint32_t adc34_packed[ADC_RESULT_BUFFER_LENGTH];
static uint32_t adc5_result;
volatile uint8_t adc_data_ready = 0;
static TIM_HandleTypeDef *tim_sample = &htim3;
// static ADC_HandleTypeDef *adcs[4] = {&hadc1, &hadc2, &hadc3, &hadc4};
// // 不再使用（多模触发重构） ADC1 - PA0 - CH1; ADC2 - PA6 - CH2; ADC3 - PB1 -
// CH3; ADC4 - PB15 - CH4; ADC5 - PA8 - VREF; 时间戳（单位：us，来自
// HAL_GetTick） 使用微秒时间戳（DWT cycle counter -> microseconds）
static volatile uint64_t ts_pulse_dma_end = 0; // DMA 传输完成时间戳
static volatile uint64_t ts_adc_start = 0;     // ADC 启动时间戳
static volatile uint64_t ts_adc_conv_end = 0;  // ADC 转换完成时间戳

static uint32_t dwt_cycles_per_us = 0;
static uint8_t dwt_ready = 0; // 0: not ready; 1: enabled and counting

// 初始化 DWT CYCCNT，用于微秒级时间戳；在 system 初始化后调用一次
void ADC_Timestamp_Init(void) {
  // 确保跟踪和 DWT 被启用
  if ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) == 0) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  }
  // 某些内核实现需要解锁 DWT（若不存在该寄存器，写入无害）
  // *((volatile uint32_t*)0xE0001FB0UL) = 0xC5ACCE55UL; // DWT->LAR unlock key
  // 清零并启用 CYCCNT
  DWT->CYCCNT = (uint32_t)0u;
  DWT->CTRL |= (uint32_t)DWT_CTRL_CYCCNTENA_Msk;
  // 计算每微秒的周期数
  if (SystemCoreClock >= 1000000U) {
    dwt_cycles_per_us = SystemCoreClock / 1000000U;
  } else {
    dwt_cycles_per_us = 1;
  }
  // 简单自检：确认计数器在递增
  uint32_t c0 = DWT->CYCCNT;
  for (volatile int i = 0; i < 1024; ++i) {
    __NOP();
  }
  uint32_t c1 = DWT->CYCCNT;
  dwt_ready = (c1 != c0) ? 1 : 0;
}

static inline uint64_t DWT_GetMicroseconds(void) {
  if (!dwt_ready) {
    // 退化：若 DWT 无法启用，退回到 ms 级时间（不会很准，但可用）
    return (uint64_t)HAL_GetTick() * 1000ULL;
  }
  uint32_t cycles = DWT->CYCCNT;
  if (dwt_cycles_per_us == 0)
    return 0;
  return (uint64_t)cycles / (uint64_t)dwt_cycles_per_us;
}

// 记录三路 DMA 完成标志；等全部完成后再合并/拆包并置 adc_data_ready
static volatile uint8_t adc12_done = 0;
static volatile uint8_t adc34_done = 0;
static volatile uint8_t adc5_done = 0;
// Task to notify when a full set of ADC data is ready
static TaskHandle_t adc_notify_task = NULL;

// 初始化：预置并启动 DMA，等待外部触发（TIMx_TRGO）
void ADC_Sync_Init(void) {
  // 预置 ADC1+ADC2 多模 DMA（主 ADC1）
  HAL_ADCEx_MultiModeStart_DMA(&hadc1, adc12_packed, ADC_RESULT_BUFFER_LENGTH);
  // 预置 ADC3+ADC4 多模 DMA（主 ADC3）
  HAL_ADCEx_MultiModeStart_DMA(&hadc3, adc34_packed, ADC_RESULT_BUFFER_LENGTH);
  // 预置 ADC5 单独 DMA
  // HAL_ADC_Start_DMA(&hadc5, (uint32_t *)adc_result[4],
                    // ADC_RESULT_BUFFER_LENGTH);
  HAL_ADC_Start_DMA(&hadc5, &adc5_result, 1);
  // 完成后，三路均处于“就绪等待触发”状态
  ADC_Timestamp_Init();
}

// 启动采样定时器，周期性产生 TRGO（CubeMX 已配置 TIMx TRGO=Update）
void ADC_Sync_Start(void)
{
  HAL_TIM_Base_Start(tim_sample);
  ts_adc_start = DWT_GetMicroseconds();
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  //*/
  // 根据完成的 ADC 标记对应完成标志
  if (hadc == &hadc1) {
    adc12_done = 1;
  } else if (hadc == &hadc3) {
    adc34_done = 1;
  } else if (hadc == &hadc5) {
    adc5_done = 1;
  }

  if (adc12_done && adc34_done && adc5_done) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE; // 通知
    ts_adc_conv_end = DWT_GetMicroseconds();
    HAL_TIM_Base_Stop(tim_sample);
    __HAL_TIM_SET_COUNTER(tim_sample, 0);

    // 标记数据就绪（具体数据拆包在任务中完成以减少 ISR 时间）
    adc_data_ready = 1;

    // 重置完成标志，等待任务处理
    adc12_done = adc34_done = adc5_done = 0;

    // 如果已注册接收任务，则从 ISR 通知（任务将做解包与发送）
    if (adc_notify_task != NULL) {
      /* Increment the task's notification value so the task can use
       * ulTaskNotifyTake() to block until a notification arrives. */
      xTaskNotifyFromISR(adc_notify_task, 0, eIncrement, &xHigherPriorityTaskWoken);
      portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // 切换任务
    }
  }
  //*/
}

void ADC_Sync_TransmitResultsIfFull(void) {
  if (adc_data_ready) {
    // 清标志，处理数据
    adc_data_ready = 0;

    // 拆包 packed buffers -> adc_result[0..3]
    for (uint16_t i = 0; i < ADC_RESULT_BUFFER_LENGTH; ++i) {
      adc_result[0][i] = (int16_t)(adc12_packed[i] & 0xFFFF) - (int16_t)(adc5_result & 0xFFFF);
      adc_result[1][i] = (int16_t)((adc12_packed[i] >> 16) & 0xFFFF) - (int16_t)(adc5_result & 0xFFFF);
      adc_result[2][i] = (int16_t)(adc34_packed[i] & 0xFFFF) - (int16_t)(adc5_result & 0xFFFF);
      adc_result[3][i] = (int16_t)((adc34_packed[i] >> 16) & 0xFFFF) - (int16_t)(adc5_result & 0xFFFF);
    }

    const int16_t *chans[4] = {adc_result[0], adc_result[1], adc_result[2], adc_result[3]};
    // 批量发送：例如每16行拼接一次
    // 将时间戳与间隔包含在 USB 帧内发送
    USB_ADC_Results_Transmit5(chans, ADC_RESULT_BUFFER_LENGTH);

    // 重新启动 DMA 以准备下一轮采样
    HAL_ADCEx_MultiModeStart_DMA(&hadc1, adc12_packed, ADC_RESULT_BUFFER_LENGTH);
    HAL_ADCEx_MultiModeStart_DMA(&hadc3, adc34_packed, ADC_RESULT_BUFFER_LENGTH);
    // HAL_ADC_Start_DMA(&hadc5, (uint32_t *)adc_result[4], ADC_RESULT_BUFFER_LENGTH);
  }
}

void ADC_Sync_SetNotifyTask(TaskHandle_t task) {
  adc_notify_task = task;
}

// Start ADC sampling immediately (called from task context after pulse DMA completes)
void ADC_Sync_StartSamplingTimer(void) {
  ts_adc_start = DWT_GetMicroseconds();
  // 启动采样定时器，周期性产生 TRGO 触发 ADC
  __HAL_TIM_SET_COUNTER(tim_sample, 0);
  HAL_TIM_Base_Start(tim_sample);
}

void MAX14808_PulseGenerator_DMATransferComplete(DMA_HandleTypeDef *hdma) {
  // 脉冲序列（一次 DMA 传输）结束 -> 通知 ADCTask 启动采样
  ts_pulse_dma_end = DWT_GetMicroseconds();
  
  if (adc_notify_task != NULL) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // 通知 ADCTask 可以启动 ADC 采样
    xTaskNotifyFromISR(adc_notify_task, 0, eIncrement, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}


// 发送缓冲（单行最大: 5 * (sign + up to 6 digits) + 4 spaces + ";\n" ≈ 5*7 + 6
// = 41 字节） 为安全与兼容 CDC 64 字节包，使用 128 字节行缓冲
#define USB_TX_LINE_BUF_SIZE 512
// 批量发送的帧缓冲尺寸（建议 >= batch_lines * 每行均值），可按需调整
#define USB_TX_FRAME_BUF_SIZE 8192

// 单次总缓冲，为避免一次性内存过大，这里逐行发送
static uint8_t line_buf[USB_TX_LINE_BUF_SIZE];
static uint8_t frame_buf[USB_TX_FRAME_BUF_SIZE];

const char sp = ' ';
const char comma = ',';
// USBD_OK: 0; USBD_BUSY: 1
bool USB_ADC_Results_Transmit5(const int16_t *const results[5],
                               uint16_t length) {
  if (!results || ADCQueueHandle == NULL)
    return false;
  for (int i = 0; i < 5; ++i) {
    if (results[i] == NULL)
      return false;
  }

  uint16_t frame_used = 0;

#define APPEND_TO_FRAME(ptr, len)                                              \
  do {                                                                         \
    uint16_t need = (uint16_t)(len);                                           \
    if (need > USB_TX_FRAME_BUF_SIZE)                                          \
      return false;                                                            \
    if ((frame_used + need) > USB_TX_FRAME_BUF_SIZE) {                         \
      return false;                                                            \
    }                                                                          \
    memcpy(&frame_buf[frame_used], (ptr), need);                               \
    frame_used = (uint16_t)(frame_used + need);                                \
  } while (0)

  // 通道逐段输出（仍用文本格式，便于上位机解析）
  for (int ch = 0; ch < 5; ++ch) {
    for (uint16_t i = 0; i < length; ++i) {
      int n = snprintf((char *)line_buf, USB_TX_LINE_BUF_SIZE, "%d",
                       (int)results[ch][i]);
      if (n <= 0 || n >= USB_TX_LINE_BUF_SIZE)
        return false;
      APPEND_TO_FRAME(line_buf, (uint16_t)n);
      if (i + 1 < length) {
        APPEND_TO_FRAME(&sp, 1);
      }
    }
    APPEND_TO_FRAME(&comma, 1); // 通道段间添加逗号
  }

  const char newline = '\n';
  APPEND_TO_FRAME(&newline, 1);

#undef APPEND_TO_FRAME

  uint16_t payload_len = frame_used;
  uint16_t total_len = (uint16_t)(payload_len + 1 + 1 + 2 + 2); // header+type+len(2)+tail(2)

  ADCUsbPacket *packet = (ADCUsbPacket *)pvPortMalloc(sizeof(ADCUsbPacket));
  if (packet == NULL)
    return false;

  packet->buf = (uint8_t *)pvPortMalloc(total_len);
  if (packet->buf == NULL) {
    vPortFree(packet);
    return false;
  }
  packet->len = total_len;

  uint8_t *p = packet->buf;
  p[0] = USB_PKT_HEADER;
  p[1] = USB_PKT_TYPE_ADC;
  p[2] = (uint8_t)(payload_len & 0xFF);      // lenL
  p[3] = (uint8_t)((payload_len >> 8) & 0xFF); // lenH
  memcpy(&p[4], frame_buf, payload_len);
  p[4 + payload_len] = USB_PKT_TAIL0;
  p[5 + payload_len] = USB_PKT_TAIL1;

  ADCUsbPacket *to_send = packet;
  osStatus_t qres = osMessageQueuePut(ADCQueueHandle, &to_send, 0, 0);
  if (qres != osOK) {
    vPortFree(packet->buf);
    vPortFree(packet);
    return false;
  }

  return true;
}
