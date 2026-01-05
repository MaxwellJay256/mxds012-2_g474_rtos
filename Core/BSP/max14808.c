/**
 * @file max14808.c
 * @author Maxwell Jay (@MaxwellJay256）
 * @brief 超声脉冲发生器 MAX14808 驱动, 支持裸机或 FreeRTOS 环境下的 DMA 脉冲输出.
 * @version 0.1
 * @date 2025-10-31
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "max14808.h"
#include "main.h"
#include "stm32g4xx_hal_gpio.h"
#include "stm32g4xx_hal_tim.h"

static TIM_HandleTypeDef *tim_pulse = NULL;
static TIM_HandleTypeDef *tim_trigger = NULL; 
static DMA_HandleTypeDef *dma_pulse = NULL;

static uint16_t default_pulse_wave[] = {
  PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  PULSE_VPP, PULSE_VNN, PULSE_VPP, PULSE_VNN,
  PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  PULSE_RECEIVE,
  // PULSE_OFF,
};

static uint16_t pulse_wave_1[] = {
  // PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  // PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  // PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  // PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  // PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  // PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF, PULSE_OFF,
  PULSE_RECEIVE,
  PULSE_VPP, PULSE_VNN, PULSE_VPP, PULSE_VNN,
  // PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE,
  // PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE,
  // PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE,
  // PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE,
  // PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE,
  // PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE, PULSE_RECEIVE,
  PULSE_OFF,
};

static uint16_t *current_pulse_pattern = default_pulse_wave;
static uint16_t current_pattern_length = sizeof(default_pulse_wave) / sizeof(default_pulse_wave[0]);

/**
 * @brief 重置 MAX14808 至初始状态.
 * 初始状态包括：所有通道设置为 SHUTDOWN、所有 DIN 引脚拉低、电流限制设置为最低（500mA）.
 */
void MAX14808_Reset()
{
  // Set mode to shutdown
  MAX14808_SetMode(MODE_SHUTDOWN);

  // Set all channels off
  for (uint8_t i = 0; i < 8; i++)
  {
    MAX14808_SetChannel(i, OFF);
  }

  // Set current limit to minimum (500mA)
  MAX14808_SetCurrentLimit(CURRENT_LIMIT_500mA);
}

/**
 * @brief 设置 MAX14808 的工作模式.
 * 
 * @param mode 详见 MAX14808_Mode 枚举类型.
 */
void MAX14808_SetMode(MAX14808_Mode mode)
{
  // mode 为十进制的 0 - 3, 拆分为二进制的 0 - 3
  uint8_t mode_bits[2] = {mode & 0x01, (mode & 0x02) >> 1};
  HAL_GPIO_WritePin(MODE0_GPIO_Port, MODE0_Pin, mode_bits[0]);
  HAL_GPIO_WritePin(MODE1_GPIO_Port, MODE1_Pin, mode_bits[1]);
}

/**
 * @brief 设置 MAX14808 的通道状态.
 * 该函数通过 HAL_GPIO 控制引脚, 不适合用来发送脉冲, 建议仅用于初始化.
 * @param channel 通道号, 范围 1 - 4.
 * @param state 通道状态, 详见 MAX14808_ChannelState 枚举类型.
 */
void MAX14808_SetChannel(uint8_t channel, uint8_t state)
{
  // state 为十进制的 0 - 3, 拆分为二进制的 0 - 3
  uint8_t state_bits[2] = {state & 0x01, (state & 0x02) >> 1};
  // channel 为十进制的 1 - 4, 每个 channel 都有 2 个引脚 INN 和 INP, 
  // 例如 channel 2 对应 GPIO_PIN_2(0x0004) 和 GPIO_PIN_3(0x0008)
  // 则应将 0x0001 移位 2 * (channel - 1) 和 2 * (channel - 1) + 1 位
  HAL_GPIO_WritePin(PULSE_GPIO_Port, (uint16_t)(0x0001 << (2 * (channel - 1))), state_bits[0]);
  HAL_GPIO_WritePin(PULSE_GPIO_Port, (uint16_t)(0x0001 << (2 * (channel - 1) + 1)), state_bits[1]);
}

/**
 * @brief 设置 MAX14808 的电流限制.
 * 
 * @param limit 详见 MAX14808_CurrentLimit 枚举类型.
 */
void MAX14808_SetCurrentLimit(MAX14808_CurrentLimit limit)
{
  // limit 为十进制的 0 - 3, 拆分为二进制的 0 - 3
  uint8_t limit_bits[2] = {limit & 0x01, (limit & 0x02) >> 1};
  HAL_GPIO_WritePin(CC0_GPIO_Port, CC0_Pin, limit_bits[0]);
  HAL_GPIO_WritePin(CC1_GPIO_Port, CC1_Pin, limit_bits[1]);
}

/**
 * @brief 初始化软件脉冲发生器, 定义 MAX14808 所需的定时器和 DMA 句柄.
 * 
 * @param htim_pulse 脉冲定时器句柄, 即用于翻转 IO 的定时器.
 * @param htim_trigger 触发定时器句柄, 即用于确定多久发送一帧脉冲的定时器.
 * @param hdma_pulse 脉冲 DMA 句柄, 即用于传输脉冲序列的 DMA.
 */
void MAX14808_PulseGenerator_Init(TIM_HandleTypeDef *htim_pulse, TIM_HandleTypeDef *htim_trigger, DMA_HandleTypeDef *hdma_pulse)
{
  tim_pulse = htim_pulse;
  tim_trigger = htim_trigger;
  dma_pulse = hdma_pulse;

  if (dma_pulse) {
    dma_pulse->XferCpltCallback = PulseDMATransferComplete;
    dma_pulse->XferErrorCallback = PulseDMATransferError;
  }
}

/**
 * @brief 启动触发定时器.
 * 
 */
void MAX14808_PulseGenerator_Start(void)
{
  if (tim_trigger != NULL) {
    HAL_TIM_Base_Start_IT(tim_trigger);
  }
}

/**
 * @brief 停止触发定时器, 脉冲定时器和脉冲的 DMA 传输.
 * 
 */
void MAX14808_PulseGenerator_Stop(void)
{
  if (tim_trigger != NULL) {
    HAL_TIM_Base_Stop_IT(tim_trigger);
  }
  
  if (tim_pulse != NULL) {
    __HAL_TIM_DISABLE_DMA(tim_pulse, TIM_DMA_UPDATE);
    HAL_TIM_Base_Stop(tim_pulse);
  }
  
  if (dma_pulse != NULL) {
    HAL_DMA_Abort(dma_pulse);
  }

  HAL_GPIO_WritePin(LED_GPIO_Port, LED_B_Pin, GPIO_PIN_SET);
}

/**
 * @brief 设置要使用的脉冲波形.
 * 
 * @param pattern 
 * @param length 
 */
void MAX14808_PulseGenerator_SetPattern(uint16_t *pattern, uint16_t length)
{
  current_pulse_pattern = pattern;
  current_pattern_length = length;
}

/**
 * @brief 内部函数, 启动脉冲序列传输.
 * 在触发定时器 ISR 或 RTOS 任务中均可调用.
 */
static void MAX14808_PulseGenerator_StartSequence(void)
{
  if (tim_pulse == NULL || dma_pulse == NULL) {
    return;
  }

  // Ensure any previous DMA/timer activity is stopped before starting new sequence
  __HAL_TIM_DISABLE_DMA(tim_pulse, TIM_DMA_UPDATE);
  HAL_TIM_Base_Stop(tim_pulse);
  __HAL_TIM_SET_COUNTER(tim_pulse, 0);
  HAL_DMA_Abort(dma_pulse);

  // Start new DMA transfer to GPIO ODR and enable timer DMA requests
  HAL_DMA_Start(dma_pulse,
                 (uint32_t)current_pulse_pattern,
                 (uint32_t)&PULSE_GPIO_Port->ODR,
                 current_pattern_length);
  __HAL_TIM_ENABLE_DMA(tim_pulse, TIM_DMA_UPDATE);
  HAL_TIM_Base_Start(tim_pulse);

  // Toggle indicator LED
  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_B_Pin);
}

/**
 * @brief Called from a FreeRTOS task context to trigger a pulse sequence.
 * This performs the same actions as the trigger IRQ handler's sequence-start
 * but is safe to call from a thread. If your application still uses the
 * hardware trigger interrupt, the IRQ handler will call the same internal
 * implementation so both methods are compatible.
 * 
 */
void MAX14808_PulseGenerator_TriggerFromTask(void)
{
  MAX14808_PulseGenerator_StartSequence();
}

void MAX14808_PulseGenerator_IRQHandler(TIM_HandleTypeDef *htim)
{
  if (htim == tim_trigger) {
    // Delegate to the shared implementation so both IRQ and task paths behave
    // the same. The internal helper checks for NULL handles.
    MAX14808_PulseGenerator_StartSequence();
  }
}
