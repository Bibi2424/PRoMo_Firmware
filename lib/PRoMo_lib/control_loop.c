#include <stdint.h>
#include <stdbool.h>

#include "utils.h"
#include "main.h"

#include "control_loop.h"
#include "pid_controller.h"
#include "encoder.h"
#include "motor.h"


extern void control_loop_run(control_loop_t* control) {
    float now = control->get_time();
    // TODO: Input filtering

    float feedback = control->get_feedback(control->id);

    control->pid.compute_interval = now - control->last_time_run;
    float output = pid_compute(&control->pid, control->target, feedback);

    //! Clamp output
    if(output > control->max_output) { output = control->max_output; }
    else if(output < -control->max_output) { output = -control->max_output; }
    else if(output > 0.0f && output < control->min_output) { output = 0.0f; }
    else if(output < 0.0f && output > -control->min_output) { output = 0.0f; }

    control->set_output(control->id, output);

    control->last_output = output;
    control->last_feedback = feedback;
    control->last_time_run = now;
}


extern void set_target(control_loop_t* control, float target) {
    control->target = target;
}

