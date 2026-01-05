/**
 * @file lm51551.h
 * @author Maxwell Jay (@MaxwellJay256)
 * @brief 
 * @version 0.1
 * @date 2026-01-04
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef LM51551_H
#define LM51551_H

#include "main.h"

void HV_Enable(void);
void HV_Disable(void);
uint8_t is_HV_Enabled(void);
uint8_t is_HV_PGood(void);

#endif /* LM51551_H */