/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cmsis_os2.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern DMA_HandleTypeDef hdma_tim2_up;
extern osMessageQueueId_t sysControlQueueHandle;
extern osSemaphoreId_t sysStateSemHandle;
extern osEventFlagsId_t sysEventGroupHandle;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PULSE_2N_Pin GPIO_PIN_2
#define PULSE_2N_GPIO_Port GPIOE
#define PULSE_2P_Pin GPIO_PIN_3
#define PULSE_2P_GPIO_Port GPIOE
#define PULSE_3N_Pin GPIO_PIN_4
#define PULSE_3N_GPIO_Port GPIOE
#define PULSE_3P_Pin GPIO_PIN_5
#define PULSE_3P_GPIO_Port GPIOE
#define PULSE_4N_Pin GPIO_PIN_6
#define PULSE_4N_GPIO_Port GPIOE
#define VCC_EN_Pin GPIO_PIN_13
#define VCC_EN_GPIO_Port GPIOC
#define ADC_CH1_Pin GPIO_PIN_0
#define ADC_CH1_GPIO_Port GPIOA
#define LED_R_Pin GPIO_PIN_2
#define LED_R_GPIO_Port GPIOA
#define LED_B_Pin GPIO_PIN_3
#define LED_B_GPIO_Port GPIOA
#define LED_G_Pin GPIO_PIN_4
#define LED_G_GPIO_Port GPIOA
#define ADC_CH2_Pin GPIO_PIN_6
#define ADC_CH2_GPIO_Port GPIOA
#define ADC_CH3_Pin GPIO_PIN_1
#define ADC_CH3_GPIO_Port GPIOB
#define I2C_INA231_SMBA_Pin GPIO_PIN_2
#define I2C_INA231_SMBA_GPIO_Port GPIOB
#define PULSE_4P_Pin GPIO_PIN_7
#define PULSE_4P_GPIO_Port GPIOE
#define PULSE_5N_Pin GPIO_PIN_8
#define PULSE_5N_GPIO_Port GPIOE
#define PULSE_5P_Pin GPIO_PIN_9
#define PULSE_5P_GPIO_Port GPIOE
#define PULSE_6N_Pin GPIO_PIN_10
#define PULSE_6N_GPIO_Port GPIOE
#define PULSE_6P_Pin GPIO_PIN_11
#define PULSE_6P_GPIO_Port GPIOE
#define PULSE_7N_Pin GPIO_PIN_12
#define PULSE_7N_GPIO_Port GPIOE
#define PULSE_7P_Pin GPIO_PIN_13
#define PULSE_7P_GPIO_Port GPIOE
#define PULSE_8N_Pin GPIO_PIN_14
#define PULSE_8N_GPIO_Port GPIOE
#define PULSE_8P_Pin GPIO_PIN_15
#define PULSE_8P_GPIO_Port GPIOE
#define ADC_CH4_Pin GPIO_PIN_15
#define ADC_CH4_GPIO_Port GPIOB
#define I2C_INA231_SCL_Pin GPIO_PIN_8
#define I2C_INA231_SCL_GPIO_Port GPIOC
#define ADC_CHREF_Pin GPIO_PIN_8
#define ADC_CHREF_GPIO_Port GPIOA
#define VCP_TX_Pin GPIO_PIN_9
#define VCP_TX_GPIO_Port GPIOA
#define VCP_RX_Pin GPIO_PIN_10
#define VCP_RX_GPIO_Port GPIOA
#define I2C_OLED_SCL_Pin GPIO_PIN_15
#define I2C_OLED_SCL_GPIO_Port GPIOA
#define I2C_INA231_SDA_Pin GPIO_PIN_11
#define I2C_INA231_SDA_GPIO_Port GPIOC
#define MODE1_Pin GPIO_PIN_6
#define MODE1_GPIO_Port GPIOD
#define MODE0_Pin GPIO_PIN_7
#define MODE0_GPIO_Port GPIOD
#define CC1_Pin GPIO_PIN_4
#define CC1_GPIO_Port GPIOB
#define CC0_Pin GPIO_PIN_5
#define CC0_GPIO_Port GPIOB
#define HV_PG_Pin GPIO_PIN_6
#define HV_PG_GPIO_Port GPIOB
#define I2C_OLED_SDA_Pin GPIO_PIN_7
#define I2C_OLED_SDA_GPIO_Port GPIOB
#define KEY_BOOT0_Pin GPIO_PIN_8
#define KEY_BOOT0_GPIO_Port GPIOB
#define HV_EN_Pin GPIO_PIN_9
#define HV_EN_GPIO_Port GPIOB
#define PULSE_1N_Pin GPIO_PIN_0
#define PULSE_1N_GPIO_Port GPIOE
#define PULSE_1P_Pin GPIO_PIN_1
#define PULSE_1P_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */
#define PULSE_GPIO_Port GPIOE
#define LED_GPIO_Port GPIOA
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
