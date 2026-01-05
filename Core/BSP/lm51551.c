/**
 * @file lm51551.c
 * @author Maxwell Jay (@MaxwellJay256)
 * @brief LM51551 控制模块
 * @version 0.1
 * @date 2026-01-04
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "lm51551.h"

/**
 * @brief 使能双级高压电源
 * 
 */
void HV_Enable(void)
{
    HAL_GPIO_WritePin(VCC_EN_GPIO_Port, VCC_EN_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(HV_EN_GPIO_Port, HV_EN_Pin, GPIO_PIN_SET);
}

/**
 * @brief 关闭双极高压电源
 * 
 */
void HV_Disable(void)
{
    HAL_GPIO_WritePin(HV_EN_GPIO_Port, HV_EN_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(VCC_EN_GPIO_Port, VCC_EN_Pin, GPIO_PIN_RESET);
}

/**
 * @brief 是否使能双极高压电源
 * 
 * @return uint8_t 1 - Enabled; 0 - Disabled
 */
uint8_t is_HV_Enabled(void)
{
    return HAL_GPIO_ReadPin(HV_EN_GPIO_Port, HV_EN_Pin);
}

/**
 * @brief 检查 LM51551 的 PGood 状态
 * 
 * @return uint8_t 1 - Power Good; 0 - Not Good
 */
uint8_t is_HV_PGood(void)
{
    return HAL_GPIO_ReadPin(HV_PG_GPIO_Port, HV_PG_Pin);
}

