/**
 * @file max14808.h
 * @author Maxwell Jay (@MaxwellJay256)
 * @brief 
 * @version 0.1
 * @date 2025-12-31
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef _MAX14808_H_
#define _MAX14808_H_

#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_gpio.h"
#include "stm32g4xx_hal_dma.h"
#include "gpio.h"
#include "adc_sync.h"

typedef enum MAX14808_Mode_e
{
  MODE_SHUTDOWN = 0, // 所有通道关断
  MODE_OCTAL_THREE_LEVEL, // 8 通道 3 电平
  MODE_QUAD_FIVE_LEVEL, // 4 通道 5 电平
  MODE_TRANSMIT_DISABLE // 禁止发送脉冲，仅打开接收
} MAX14808_Mode;

typedef enum MAX14808_ChannelState_e
{
  OFF = 0,
  VNN,
  VPP,
  RECEIVE
} MAX14808_ChannelState;

typedef enum MAX14808_CurrentLimit_e
{
  CURRENT_LIMIT_2000mA = 0,
  CURRENT_LIMIT_1500mA,
  CURRENT_LIMIT_1000mA,
  CURRENT_LIMIT_500mA
} MAX14808_CurrentLimit;

// 需要注意，MXDS012-2 只连接了 1 - 4 通道，5 - 8 通道禁止使用
#define PULSE_OFF 0x0000
#define PULSE_VPP 0x00AA // 0000 0000 1010 1010
#define PULSE_VNN 0x0055 // 0000 0000 0101 0101
#define PULSE_RECEIVE 0x00FF // 0000 0000 1111 1111, 0d255

void MAX14808_Reset();
void MAX14808_SetMode(MAX14808_Mode mode);
void MAX14808_SetChannel(uint8_t channel, uint8_t state);
void MAX14808_SetCurrentLimit(MAX14808_CurrentLimit limit);

void MAX14808_PulseGenerator_Init(TIM_HandleTypeDef *htim_pulse, TIM_HandleTypeDef *htim_trigger, DMA_HandleTypeDef *hdma);
void MAX14808_PulseGenerator_Start(void);
void MAX14808_PulseGenerator_Stop(void);
void MAX14808_PulseGenerator_SetPattern(uint16_t *pattern, uint16_t length);
void MAX14808_PulseGenerator_IRQHandler(TIM_HandleTypeDef *htim);

void MAX14808_PulseGenerator_TriggerFromTask(void);

#endif // _MAX14808_H_