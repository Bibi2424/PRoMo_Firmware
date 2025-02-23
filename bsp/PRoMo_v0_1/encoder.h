#ifndef ENCODER_H
#define ENCODER_H

#include "bsp.h"

extern void encoders_init(void);
extern uint32_t encoder_get_tick_count(actuator_t id);

#endif