#include "led.h"
#include "stm32g4xx_hal_gpio.h"

/**
 * @brief 将 LED 设为指定颜色，其余颜色熄灭
 * 
 * @param color 
 */
void LED_SetColor(LEDColor color)
{
    switch (color) {
        case OFF:
            HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_SET);
            break;
        case RED:
            HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_SET);
            break;
        case GREEN:
            HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_SET);
            break;
        case BLUE:
            HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
            HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_RESET);
            break;
        case WHITE:
            HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_RESET);
            break;
        default:
            break;
    }
}

/**
 * @brief 使能 LED 指定颜色，但不改变其余颜色状态
 * 
 * @param color 
 */
void LED_EnableColor(LEDColor color)
{
    switch (color) {
        case OFF:
            break;
        case RED:
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);
            break;
        case GREEN:
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
            break;
        case BLUE:
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_B_Pin, GPIO_PIN_RESET);
            break;
        case WHITE:
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_G_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_B_Pin, GPIO_PIN_RESET);
            break;
        default:
            break;
    }
}

/**
 * @brief 熄灭 LED 指定颜色，但不改变其余颜色状态
 * 
 * @param color 
 */
void LED_ResetColor(LEDColor color)
{
    switch (color) {
        case RED:
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);
            break;
        case GREEN:
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_G_Pin, GPIO_PIN_SET);
            break;
        case BLUE:
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_B_Pin, GPIO_PIN_SET);
            break;
        default:
            break;
    }
}

/**
 * @brief 翻转 LED 指定颜色亮灭，但不改变其余颜色状态
 * 
 * @param color 
 */
void LED_ToggleColor(LEDColor color)
{
    switch (color) {
        case OFF:
            break;
        case RED:
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_R_Pin);
            break;
        case GREEN:
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_G_Pin);
            break;
        case BLUE:
            HAL_GPIO_TogglePin(LED_GPIO_Port, LED_B_Pin);
            break;
        default:
            break;
    }
}
