#ifndef PID_H
#define PID_H

#include "main.h"

typedef struct {
	uint32_t compute_interval;
	uint32_t proportional_gain;
	uint32_t integral_gain;
	uint32_t derivative_gain;
	int32_t last_error;
	int32_t integral_error;
	// uint32_t max;
	// uint32_t min;

} pid_controller_t;


extern int32_t pid_compute(pid_controller_t *pid, int32_t set_point, int32_t current_value);


#endif