#ifndef DRIVE_SPEED_CONTROL_H
#define DRIVE_SPEED_CONTROL_H


#include "control_loop.h"


extern void drive_speed_control_init(void);
extern void drive_speed_control_loop(void);
extern void set_target_speed_percent(int32_t target_left, int32_t target_right);

extern void update_speed_pid(actuator_t side, float p, float i, float d);


#endif
