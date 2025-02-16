#ifndef PID_H
#define PID_H


typedef struct {
	float proportional_gain;
	float integral_gain;
	float derivative_gain;
	//! Internal stuff
	float last_run;
	float last_error;
	float integral_error;
} pid_controller_t;


extern float pid_compute(pid_controller_t *pid, float set_point, float current_value, bool is_clamped);
extern void pid_reset(pid_controller_t *pid);

#endif