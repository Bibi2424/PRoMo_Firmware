#ifndef PID_H
#define PID_H


typedef struct {
	float compute_interval;
	float proportional_gain;
	float integral_gain;
	float derivative_gain;
	//! Internal stuff
	float last_error;
	float integral_error;
} pid_controller_t;


extern float pid_compute(pid_controller_t *pid, float set_point, float current_value);
extern void pid_reset(pid_controller_t *pid);

#endif