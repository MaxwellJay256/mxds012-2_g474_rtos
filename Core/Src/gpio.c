/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
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
#include "gpio.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, PULSE_2N_Pin|PULSE_2P_Pin|PULSE_3N_Pin|PULSE_3P_Pin
                          |PULSE_4N_Pin|PULSE_4P_Pin|PULSE_5N_Pin|PULSE_5P_Pin
                          |PULSE_6N_Pin|PULSE_6P_Pin|PULSE_7N_Pin|PULSE_7P_Pin
                          |PULSE_8N_Pin|PULSE_8P_Pin|PULSE_1N_Pin|PULSE_1P_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(VCC_EN_GPIO_Port, VCC_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LED_R_Pin|LED_B_Pin|LED_G_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, MODE1_Pin|MODE0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, CC1_Pin|CC0_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(HV_EN_GPIO_Port, HV_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PULSE_2N_Pin PULSE_2P_Pin PULSE_3N_Pin PULSE_3P_Pin
                           PULSE_4N_Pin PULSE_4P_Pin PULSE_5N_Pin PULSE_5P_Pin
                           PULSE_6N_Pin PULSE_6P_Pin PULSE_7N_Pin PULSE_7P_Pin
                           PULSE_8N_Pin PULSE_8P_Pin PULSE_1N_Pin PULSE_1P_Pin */
  GPIO_InitStruct.Pin = PULSE_2N_Pin|PULSE_2P_Pin|PULSE_3N_Pin|PULSE_3P_Pin
                          |PULSE_4N_Pin|PULSE_4P_Pin|PULSE_5N_Pin|PULSE_5P_Pin
                          |PULSE_6N_Pin|PULSE_6P_Pin|PULSE_7N_Pin|PULSE_7P_Pin
                          |PULSE_8N_Pin|PULSE_8P_Pin|PULSE_1N_Pin|PULSE_1P_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : VCC_EN_Pin */
  GPIO_InitStruct.Pin = VCC_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(VCC_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_R_Pin LED_B_Pin LED_G_Pin */
  GPIO_InitStruct.Pin = LED_R_Pin|LED_B_Pin|LED_G_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : MODE1_Pin MODE0_Pin */
  GPIO_InitStruct.Pin = MODE1_Pin|MODE0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : CC1_Pin CC0_Pin */
  GPIO_InitStruct.Pin = CC1_Pin|CC0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : HV_PG_Pin */
  GPIO_InitStruct.Pin = HV_PG_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(HV_PG_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : KEY_BOOT0_Pin */
  GPIO_InitStruct.Pin = KEY_BOOT0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(KEY_BOOT0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : HV_EN_Pin */
  GPIO_InitStruct.Pin = HV_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(HV_EN_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
