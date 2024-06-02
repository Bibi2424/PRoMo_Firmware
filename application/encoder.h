#ifndef ENCODER_H
#define ENCODER_H

#include "main.h"


#define INVERSE_LEFT_ENCODER	false
#define INVERSE_RIGHT_ENCODER	true



extern void encoders_init(void);

extern uint32_t encoder_get_tick_count(actuator_t side);
extern float encoder_get_speed(actuator_t side, float wheel_radius, float tick_per_wheel_turn_div_pi);

#endif