#define DEBUG_THIS_FILE	DEBUG_PID_FILE

#include <stdint.h>
#include <stdbool.h>

#include "global.h"
#include "debug.h"

#include "pid_controller.h"


extern int32_t pid_compute(pid_controller_t *pid, int32_t set_point, int32_t current_value) {
	int32_t output = 0;

	int32_t error = set_point - current_value;

	output += pid->proportional_gain * error;

	pid->integral_error += error;
	output += pid->integral_gain * pid->integral_error;

	output += pid->derivative_gain * (error - pid->last_error) / pid->compute_interval;
	pid->last_error = error;

	//! TODO: min/max stuff

	return output;
}
