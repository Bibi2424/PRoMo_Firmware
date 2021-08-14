#ifndef ENCODER_H
#define ENCODER_H

#include "main.h"


#define INVERSE_LEFT_ENCODER	false
#define INVERSE_RIGHT_ENCODER	true



extern void encoders_init(void);

extern uint16_t encoder_get_value(actuator_t side);
extern int16_t encoder_get_speed(actuator_t side);

#endif