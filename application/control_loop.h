#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H


#include "pid_controller.h"


#define CONTROL_LOOP_MAX_NAME_SIZE	16


typedef float (*cb_get_feedback_t)(unsigned id);
typedef void (*cb_set_output_t)(unsigned id, float output);
typedef float (*cb_get_time_t)(void);


typedef struct {
	char name[CONTROL_LOOP_MAX_NAME_SIZE];
	unsigned id;

	float target;

	float min_output;
	float max_output;

	pid_controller_t pid;

	cb_get_feedback_t get_feedback;
	cb_set_output_t set_output;
	cb_get_time_t get_time;

	//Internal
	float last_feedback;
	float last_output;
	float last_time_run;
} control_loop_t;


extern void control_loop_init(void);
extern void do_control_loop(void);
extern void set_target_speed_percent(int32_t target_left, int32_t target_right);

extern void update_speed_pid(actuator_t side, float p, float i, float d);

#endif
