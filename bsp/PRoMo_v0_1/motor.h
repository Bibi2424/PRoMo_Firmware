#ifndef MOTOR_H
#define MOTOR_H

#include "bsp.h"

#define MOTOR_DIR_FORWARD	1
#define MOTOR_DIR_REVERSE	0
#define MOTOR_DIR_DISABLE	0xFF

#define INVERSE_MOTOR_D		false
#define INVERSE_MOTOR_G 	true


extern void motors_init(void);

extern void motor_set_dir(actuator_t side, uint8_t dir);
extern void motor_set_speed(actuator_t side, uint32_t percent);

#endif