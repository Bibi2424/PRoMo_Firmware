#include <stdint.h>
#include <stdbool.h>
#include <float.h>

#include "pid_controller.h"
// Remove dependency to time.h, pass it as argument
#include "time.h"


extern float pid_compute(pid_controller_t *pid, float set_point, float current_value) {
	float output = 0;
	float error = set_point - current_value;
	// float now = ((float)get_time_microsecond() / 1000000.0f);
	// float elapse_time = (now - pid->last_run);

	float p_term = pid->proportional_gain * error;
	
	if(pid->is_clamped == false) {
		pid->integral_error += error;
	}
	float i_term = pid->integral_gain * pid->integral_error;
	// float i_term = 0.0f;
	// if(pid->last_run <= FLT_MIN) {
	// 	i_term = pid->integral_gain * pid->integral_error * elapse_time;
	// }

	float d_term = pid->derivative_gain * (error - pid->last_error);
	// float d_term = 0.0f;
	// if(pid->last_run <= FLT_MIN) {
	// 	d_term = pid->derivative_gain * (error - pid->last_error) / elapse_time;
	// }
	pid->last_error = error;

	output = p_term + i_term + d_term;
	// pid->last_run = now;

	return output;
}


extern void pid_reset(pid_controller_t *pid) {
	pid->last_run = 0.0f;
	pid->last_error = 0.0f;
	pid->integral_error = 0.0f;
}


