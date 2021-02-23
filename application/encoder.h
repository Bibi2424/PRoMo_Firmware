#ifndef ENCODER_H
#define ENCODER_H

#include "main.h"

extern void TIM34_Encoder_Init(void);
extern uint16_t encoder_left_get_value(void);
extern uint16_t encoder_right_get_value(void);
extern int16_t encoder_left_get_speed(void);
extern int16_t encoder_right_get_speed(void);

#endif