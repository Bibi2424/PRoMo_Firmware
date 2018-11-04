#ifndef ENCODER_H
#define ENCODER_H

#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_rcc.h"
#include "main.h"

extern void TIM3_Encoder_Init(void);
extern uint16_t encoder_left_get_value(void);
extern uint16_t encoder_right_get_value(void);

#endif