#include "FreeRTOS.h"
#include "Types/SystemTypes.h"
#include "adc_sync.h"
#include "cmsis_os2.h"
#include "event_groups.h"
#include "main.h"


/**
 * @brief ADC 任务：等待脉冲发送完成信号，启动 ADC 采样并发送结果
 *
 * 流程：
 * 1. 初始化 ADC 多模 DMA
 * 2. 注册自己到 adc_sync，当脉冲 DMA 完成时会被唤醒
 * 3. 每次被唤醒后：
 *    - 等待并检查系统是否处于 Running 状态
 *    - 启动 ADC 采样
 *    - 阻塞等待 ADC 转换完成通知
 *    - 发送结果数据
 *
 * @param argument 未使用
 */
void StartADCTask(void *argument) {
  // 初始化 ADC 多模采样、DMA 等基础设施（不启动定时器）
  ADC_Sync_Init();

  // 注册当前任务，当脉冲 DMA 完成时会收到通知
  ADC_Sync_SetNotifyTask(xTaskGetCurrentTaskHandle());

  uint32_t notifyCount;
  EventBits_t sysEventbits;

  for (;;) {
    // 阻塞等待脉冲 DMA 完成通知
    // ulTaskNotifyTake 会在收到通知后返回，计数器递减
    notifyCount = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    if (notifyCount > 0) {
      // 收到脉冲完成信号，检查系统是否运行
      sysEventbits = xEventGroupGetBits(sysEventGroupHandle);

      if (sysEventbits & SYS_EVENT_RUNNING_BIT) {
        // 系统运行中，启动 ADC 采样
        // ADC_Sync_StartSampling() 会启动采样定时器
        ADC_Sync_StartSamplingTimer();

        // 继续阻塞，等待 ADC 转换完成
        // HAL_ADC_ConvCpltCallback() 会再次通知此任务
        notifyCount = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        if (notifyCount > 0) {
          // ADC 数据已准备好，拆包、发送结果并重启 DMA
          ADC_Sync_TransmitResultsIfFull();
        }
      }
      // 如果系统不运行，直接丢弃脉冲通知，循环继续等待下一次脉冲
    }
  }
}