#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H


#include "pid_controller.h"


#define CONTROL_LOOP_MAX_NAME_SIZE	16


typedef float (*cb_get_feedback_t)(unsigned id);
typedef void (*cb_set_output_t)(unsigned id, float output);


typedef struct {
	char name[CONTROL_LOOP_MAX_NAME_SIZE];
	unsigned id;

	float target;

	float max_input_derivative;
	float min_output;
	float max_output;

	pid_controller_t pid;

	cb_get_feedback_t get_feedback;
	cb_set_output_t set_output;

	//Internal
	float new_target;
	float last_feedback;
	float last_output;
	float last_time_run;
} control_loop_t;


extern void control_loop_run(control_loop_t* control, float now);
extern void set_target(control_loop_t* control, float target);


#endif
