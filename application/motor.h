#ifndef MOTOR_H
#define MOTOR_H

#include "main.h"

#define MOTOR_DIR_FORWARD	1
#define MOTOR_DIR_REVERSE	0
#define MOTOR_DIR_DISABLE	0xFF

// #define INVERSE_MOTOR_D
#define INVERSE_MOTOR_G

extern void motors_init(void);
extern void motor_right_set_dir(uint8_t dir);
extern void motor_left_set_dir(uint8_t dir);
extern void motor_right_set_speed(uint32_t speed_percent);
extern void motor_left_set_speed(uint32_t speed_percent);

#endif