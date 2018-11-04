#ifndef MOTOR_H
#define MOTOR_H

#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_rcc.h"
#include "main.h"

#define MOTOR_DIR_FORWARD	1
#define MOTOR_DIR_REVERSE	0

extern void TIM2_Motor_Init(void);
extern void motor_right_set_dir(uint8_t dir);
extern void motor_left_set_dir(uint8_t dir);
extern void motor_right_set_speed(uint32_t speed_percent);
extern void motor_left_set_speed(uint32_t speed_percent);

#endif