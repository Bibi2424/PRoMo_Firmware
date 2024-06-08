#include <stdint.h>
#include <stdbool.h>

#include "pid_controller.h"


extern float pid_compute(pid_controller_t *pid, float set_point, float current_value) {
	float output = 0;
	float error = set_point - current_value;

	float p_term = pid->proportional_gain * error;
	
	pid->integral_error += error;
	float i_term = pid->integral_gain * pid->integral_error;

	float d_term = pid->derivative_gain * (error - pid->last_error);
	pid->last_error = error;

	output = p_term + i_term + d_term;

	return output;
}


extern void pid_reset(pid_controller_t *pid) {
	pid->last_error = 0.0f;
	pid->integral_error = 0.0f;
}


