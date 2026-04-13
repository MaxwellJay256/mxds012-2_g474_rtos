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
#include "tim.h"
#include "cmsis_os.h"
#include "main.h"
#include <stdint.h>
#include <stdio.h>

// #define ENABLE_ADC_SYNC_DEBUG

#ifdef ENABLE_ADC_SYNC_DEBUG
#define ADC_SYNC_LOG(...) printf(__VA_ARGS__)
#else
#define ADC_SYNC_LOG(...)
#endif

static int16_t adc_result[4][ADC_RESULT_BUFFER_LENGTH];
// 多模打包缓冲：ADC1 + ADC2、ADC3 + ADC4 各用 32-bit 打包缓冲
static uint32_t adc12_packed[ADC_RESULT_BUFFER_LENGTH];
static uint32_t adc34_packed[ADC_RESULT_BUFFER_LENGTH];
static uint32_t adc5_result;
static uint16_t current_sample_depth = CONFIG_SAMPLE_DEPTH_MIN;
volatile uint8_t adc_data_ready = 0;
static TIM_HandleTypeDef *tim_sample = &htim3;

static volatile uint64_t ts_pulse_dma_end = 0; // DMA 传输完成时间戳
static volatile uint64_t ts_adc_start = 0;     // ADC 启动时间戳
static volatile uint64_t ts_adc_conv_end = 0;  // ADC 转换完成时间戳

static uint32_t dwt_cycles_per_us = 0;
static uint8_t dwt_ready = 0; // 0: not ready; 1: enabled and counting

static uint16_t ADC_GetConfiguredSampleDepth(SysConfig *config)
{
  uint16_t depth = config->sample_depth;

  if (depth < CONFIG_SAMPLE_DEPTH_MIN) {
    depth = CONFIG_SAMPLE_DEPTH_MIN;
  } else if (depth > CONFIG_SAMPLE_DEPTH_MAX) {
    depth = CONFIG_SAMPLE_DEPTH_MAX;
  }

  return depth;
}

/**
 * @brief 初始化 DWT CYCCNT，用于微秒级时间戳；在 system 初始化后调用一次
 * @retval None
 */
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
  current_sample_depth = ADC_GetConfiguredSampleDepth(&sysConfig);
  #ifdef ENABLE_ADC_SYNC_DEBUG
    HAL_StatusTypeDef st12 = HAL_OK;
    HAL_StatusTypeDef st34 = HAL_OK;
    HAL_StatusTypeDef st5 = HAL_OK;

    st12 = HAL_ADCEx_MultiModeStart_DMA(&hadc1, adc12_packed, current_sample_depth);
    st34 = HAL_ADCEx_MultiModeStart_DMA(&hadc3, adc34_packed, current_sample_depth);
    st5 = HAL_ADC_Start_DMA(&hadc5, &adc5_result, 1);
  #else
    HAL_ADCEx_MultiModeStart_DMA(&hadc1, adc12_packed, current_sample_depth); // 预置 ADC1 + ADC2 多模 DMA（主 ADC1）
    HAL_ADCEx_MultiModeStart_DMA(&hadc3, adc34_packed, current_sample_depth); // 预置 ADC3 + ADC4 多模 DMA（主 ADC3）
    HAL_ADC_Start_DMA(&hadc5, &adc5_result, 1); // 预置 ADC5 单独 DMA（基准电压）
  #endif // ENABLE_ADC_SYNC_DEBUG

  ADC_Timestamp_Init();
  ADC_SYNC_LOG("[adc_sync] init depth=%u st12=%d st34=%d st5=%d\n",
               (unsigned)current_sample_depth,
               (int)st12,
               (int)st34,
               (int)st5);
}

// 启动采样定时器，周期性产生 TRGO（CubeMX 已配置 TIMx TRGO=Update）
void ADC_Sync_Start(void)
{
  HAL_TIM_Base_Start(tim_sample);
  ts_adc_start = DWT_GetMicroseconds();
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
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
}

void ADC_Sync_TransmitResultsIfFull(void) {
  #ifdef ENABLE_ADC_SYNC_DEBUG
    static uint32_t frame_seq = 0U;
  #endif // ENABLE_ADC_SYNC_DEBUG

  if (adc_data_ready) {
    #ifdef ENABLE_ADC_SYNC_DEBUG
      uint8_t tx_res = 0U;
      HAL_StatusTypeDef st12 = HAL_OK;
      HAL_StatusTypeDef st34 = HAL_OK;
      HAL_StatusTypeDef st5 = HAL_OK;
    #endif // ENABLE_ADC_SYNC_DEBUG
    const uint16_t sample_depth = current_sample_depth;

    // 清标志，处理数据
    adc_data_ready = 0;

    // 拆包 packed buffers -> adc_result[0..3], 并减去基准电压 adc5_result
    for (uint16_t i = 0; i < sample_depth; ++i) {
      adc_result[0][i] = (int16_t)(adc12_packed[i] & 0xFFFF) - (int16_t)(adc5_result & 0xFFFF);
      adc_result[1][i] = (int16_t)((adc12_packed[i] >> 16) & 0xFFFF) - (int16_t)(adc5_result & 0xFFFF);
      adc_result[2][i] = (int16_t)(adc34_packed[i] & 0xFFFF) - (int16_t)(adc5_result & 0xFFFF);
      adc_result[3][i] = (int16_t)((adc34_packed[i] >> 16) & 0xFFFF) - (int16_t)(adc5_result & 0xFFFF);
    }

    const int16_t *chans[4] = {adc_result[0], adc_result[1], adc_result[2], adc_result[3]};

    #ifdef ENABLE_ADC_SYNC_DEBUG
      tx_res = USB_ADC_Transmit_Bin(chans, sample_depth);
      frame_seq++;
      ADC_SYNC_LOG("[adc_sync] frame=%lu depth=%u tx=%u q=%lu t_us=%llu\n",
                  (unsigned long)frame_seq,
                  (unsigned)sample_depth,
                  (unsigned)tx_res,
                  (unsigned long)osMessageQueueGetCount(UsbTxQueueHandle),
                  (unsigned long long)(ts_adc_conv_end - ts_adc_start));
    #else
      (void)USB_ADC_Transmit_Bin(chans, sample_depth);
    #endif // ENABLE_ADC_SYNC_DEBUG

    current_sample_depth = ADC_GetConfiguredSampleDepth(&sysConfig);

    // 重新启动 DMA 以准备下一轮采样
    #ifdef ENABLE_ADC_SYNC_DEBUG
      st12 = HAL_ADCEx_MultiModeStart_DMA(&hadc1, adc12_packed, current_sample_depth);
      st34 = HAL_ADCEx_MultiModeStart_DMA(&hadc3, adc34_packed, current_sample_depth);
      st5 = HAL_ADC_Start_DMA(&hadc5, &adc5_result, 1);
      if (st12 != HAL_OK || st34 != HAL_OK || st5 != HAL_OK) {
        ADC_SYNC_LOG("[adc_sync] DMA restart fail depth=%u st12=%d st34=%d st5=%d\n",
                    (unsigned)current_sample_depth,
                    (int)st12,
                    (int)st34,
                    (int)st5);
      }
    #else
      HAL_ADCEx_MultiModeStart_DMA(&hadc1, adc12_packed, current_sample_depth);
      HAL_ADCEx_MultiModeStart_DMA(&hadc3, adc34_packed, current_sample_depth);
      HAL_ADC_Start_DMA(&hadc5, &adc5_result, 1);
    #endif // ENABLE_ADC_SYNC_DEBUG
  } else {
    ADC_SYNC_LOG("[adc_sync] TransmitResults called but adc_data_ready=0\n");
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

/**
 * @brief 以二进制载荷格式组织 ADC 数据，发送 UsbFrame* 到 UsbTxQueue
 *
 * @param results 4 路通道数据指针（ch0..ch3）
 * @param length 每路样本数
 * @return uint8_t 0: Success, 1: Failure (e.g., memory allocation or queue error)
 */
uint8_t USB_ADC_Transmit_Bin(const int16_t *const results[4], uint16_t length)
{
  const uint16_t payload_len = 4 * length * sizeof(int16_t);
  UsbFrame *frame = NULL;
  uint8_t *payload = NULL;
  uint8_t *buf_ptr = NULL;

  if (results == NULL || UsbTxQueueHandle == NULL) {
    ADC_SYNC_LOG("[adc_sync] USB_ADC_Transmit_Bin invalid args results=%p queue=%p\n", results, UsbTxQueueHandle);
    return 1;
  }

  for (uint8_t ch = 0; ch < 4; ++ch) {
    if (results[ch] == NULL) {
      ADC_SYNC_LOG("[adc_sync] USB_ADC_Transmit_Bin NULL channel ch=%u\n", (unsigned)ch);
      return 1;
    }
  }

  frame = (UsbFrame *)pvPortMalloc(sizeof(UsbFrame));
  if (frame == NULL) {
    ADC_SYNC_LOG("[adc_sync] pvPortMalloc frame failed heap=%lu\n", (unsigned long)xPortGetFreeHeapSize());
    return 1;
  }

  payload = (uint8_t *)pvPortMalloc(payload_len);
  if (payload == NULL) {
    ADC_SYNC_LOG("[adc_sync] pvPortMalloc payload failed len=%u heap=%lu\n",
                 (unsigned)payload_len,
                 (unsigned long)xPortGetFreeHeapSize());
    vPortFree(frame);
    return 1;
  }

  buf_ptr = payload;
  for (uint8_t ch = 0; ch < 4; ++ch) {
    memcpy(buf_ptr, results[ch], length * sizeof(int16_t));
    buf_ptr += length * sizeof(int16_t);
  }

  frame->type = USB_PKT_TYPE_ADC;
  frame->payload_len = payload_len;
  frame->payload = payload;

  UsbFrame *to_send = frame;
  osStatus_t qres = osMessageQueuePut(UsbTxQueueHandle, &to_send, 0, 0);
  if (qres != osOK) {
    ADC_SYNC_LOG("[adc_sync] UsbTxQueue put failed qres=%d qcnt=%lu qspace=%lu\n",
                 (int)qres,
                 (unsigned long)osMessageQueueGetCount(UsbTxQueueHandle),
                 (unsigned long)osMessageQueueGetSpace(UsbTxQueueHandle));
    vPortFree(payload);
    vPortFree(frame);
    return 1;
  } else {
    return 0;
  }
}
