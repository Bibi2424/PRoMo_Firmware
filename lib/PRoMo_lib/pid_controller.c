#include <stdint.h>
#include <stdbool.h>

#include "pid_controller.h"
#include "time.h"


extern float pid_compute(pid_controller_t *pid, float set_point, float current_value, bool is_clamped) {
	float output = 0;
	float error = set_point - current_value;
	float now = ((float)get_time_microsecond() / 1000000.0f);
	float elapse_time = (now - pid->last_run);

	float p_term = pid->proportional_gain * error;
	
	if(is_clamped == false) {
		pid->integral_error += error;
	}
	float i_term = pid->integral_gain * pid->integral_error;
	// float i_term = pid->integral_gain * pid->integral_error * elapse_time;

	float d_term = pid->derivative_gain * (error - pid->last_error);
	// float d_term = pid->derivative_gain * (error - pid->last_error) / elapse_time;
	pid->last_error = error;

	pid->last_run = now;

	output = p_term + i_term + d_term;

	return output;
}


extern void pid_reset(pid_controller_t *pid) {
	pid->last_error = 0.0f;
	pid->integral_error = 0.0f;
}


