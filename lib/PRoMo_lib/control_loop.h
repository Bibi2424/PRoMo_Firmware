#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H


#include "pid_controller.h"
#include "odometry.h"


#define CONTROL_LOOP_MAX_NAME_SIZE	16


typedef float (*cb_get_feedback_t)(odometry_t *odo);
typedef void (*cb_set_output_t)(unsigned id, float output);


typedef struct {
	char name[CONTROL_LOOP_MAX_NAME_SIZE];
	unsigned id;

	float target;
	float next_target;

	float max_input_derivative;
	float min_output;
	float max_output;

	odometry_t odo;
	pid_controller_t pid;

	cb_get_feedback_t get_feedback;
	cb_set_output_t set_output;

	//Internal
	float filtered_target;
	float last_feedback;
	float last_output;
} control_loop_t;


extern void control_loop_run(control_loop_t* control);


#endif
