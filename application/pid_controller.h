#ifndef PID_H
#define PID_H

#include "main.h"



//! NOTE: To avoid floating point, those value will be / 100 when used
typedef struct {
	uint32_t compute_interval;
	uint32_t proportional_gain;
	uint32_t integral_gain;
	uint32_t derivative_gain;
	int32_t max_integral_error;
	uint32_t max_output;
	uint32_t min_output;
	//! Internal stuff
	int32_t last_error;
	int32_t integral_error;

} pid_controller_t;


extern int32_t pid_compute(pid_controller_t *pid, int32_t set_point, int32_t current_value);


#endif