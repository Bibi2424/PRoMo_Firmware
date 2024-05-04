#define DEBUG_THIS_FILE	DEBUG_PID_FILE

#include <stdint.h>
#include <stdbool.h>

#include "utils.h"
#include "debug.h"

#include "pid_controller.h"


extern int32_t pid_compute(pid_controller_t *pid, int32_t set_point, int32_t current_value) {
	int32_t output = 0;

	int32_t error = set_point - current_value;

	int32_t p_term = (int32_t)pid->proportional_gain * error / 100;

	pid->integral_error += error;	
	if(pid->integral_error > (pid->min_integral_error)) { pid->integral_error = (pid->min_integral_error); }
	else if(pid->integral_error < -(pid->min_integral_error)) { pid->integral_error = -(pid->min_integral_error); }
	if(error == 0 && pid->last_error == 0) { pid->integral_error = 0; }

	if(pid->integral_error > pid->max_integral_error) { pid->integral_error = pid->max_integral_error; }
	else if(pid->integral_error <  - (int32_t)pid->max_integral_error) { pid->integral_error = - (int32_t) pid->max_integral_error; }
	int32_t i_term = (int32_t)pid->integral_gain * pid->integral_error / 100;

	int32_t d_term = (int32_t)pid->derivative_gain * (error - pid->last_error) / 100;
	pid->last_error = error;

	output = p_term + i_term + d_term;

	// debugf("[%ld/%ld] - %ld - [%ld - %ld (%ld) - %ld]\n", output, set_point, error, p_term, i_term, pid->integral_error, d_term);

	if(ABS(output) < pid->min_output) { output = 0; }
	else if(ABS(output) > pid->max_output) {
		if(output > 0) { output = (int32_t)pid->max_output; }
		if(output < 0) { output = -(int32_t)pid->max_output; }
	}

	return output;
}


extern void pid_reset(pid_controller_t *pid) {
	pid->last_error = 0;
	pid->integral_error = 0;
}


