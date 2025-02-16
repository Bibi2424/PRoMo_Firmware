#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "control_loop.h"
#include "pid_controller.h"


static float control_loop_filter_input(control_loop_t* control) {
    // TODO: LERP
    if(control->max_input_derivative == NAN) {
        return control->new_target;
    }

    if(control->new_target - control->target > 0.0f) {
        if(control->new_target - control->target > control->max_input_derivative) {
            return control->target + control->max_input_derivative;
        }
        else {
            return control->new_target;
        }
    }
    else {
        if(control->new_target - control->target < -control->max_input_derivative) {
            return control->target - control->max_input_derivative;
        }
        else {
            return control->new_target;
        }
    }
}


extern void control_loop_run(control_loop_t* control, float now) {
    control->target = control_loop_filter_input(control);

    float feedback = control->get_feedback(control->id);

    // TODO: Do something about compute interval to compute derivative term
    float compute_interval = now - control->last_time_run;
    float output = pid_compute(&control->pid, control->target, feedback);

    //! Clamp output
    if(output > control->max_output) { 
        output = control->max_output; 
        control->pid.is_clamped = true;
    }
    else if(output < -control->max_output) { 
        output = -control->max_output; 
        control->pid.is_clamped = true;
    }
    else {
        control->pid.is_clamped = false;
    }
    if(output > 0.0f && output < control->min_output) { output = 0.0f; }
    else if(output < 0.0f && output > -control->min_output) { output = 0.0f; }

    control->set_output(control->id, output);

    control->last_output = output;
    control->last_feedback = feedback;
    control->last_time_run = now;
}


extern void set_target(control_loop_t* control, float target) {
    control->new_target = target;
}

