#ifndef LED_H
#define LED_H

#include "main.h"

typedef enum LEDColor_t {
    OFF = 0,
    RED = 1,
    GREEN,
    BLUE,
    WHITE,
} LEDColor;

void LED_SetColor(LEDColor color);
void LED_EnableColor(LEDColor color);
void LED_ResetColor(LEDColor color);
void LED_ToggleColor(LEDColor color);

#endif // LED_H