/**
 * @file key.c
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2026-01-02
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "key.h"

#define IS_KEY_PRESSED() (HAL_GPIO_ReadPin(KEY_BOOT0_GPIO_Port, KEY_BOOT0_Pin) == GPIO_PIN_SET)
#define KEY_CHECK_INTERVAL_MS 10 // 按键检测间隔
#define KEY_DEBOUNCE_MS 30 // 按键消抖时间
#define KEY_DEBOUNCE_COUNT (KEY_DEBOUNCE_MS / KEY_CHECK_INTERVAL_MS) // 消抖计数

uint8_t isKeyClicked(void)
{
    static uint8_t count = 0; // 消抖计数
    static uint8_t pressed = 0; // 按键状态, 用于每次按键只返回一次点击事件
    if (IS_KEY_PRESSED() && !pressed) {
        count++;
        if (count >= KEY_DEBOUNCE_COUNT && IS_KEY_PRESSED()) {
            pressed = 1;
            return 1; // 按键被点击
        }
    }

    // 按键未按下, 则重置计数和状态
    if (!IS_KEY_PRESSED()) {
        count = 0;
        pressed = 0;
    }
    return 0;
}