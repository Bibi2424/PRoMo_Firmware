#define DEBUG_THIS_FILE DEBUG_CONTROL_LOOP_FILE

#include <stdint.h>
#include <stdbool.h>

#include "utils.h"
#include "main.h"

#include "control_loop.h"
#include "pid_controller.h"
#include "encoder.h"
#include "motor.h"


#define MAX_ACCEL_PER_LOOP (MAX_ACCEL * (MOTOR_CONTROL_INTERVAL_MS / 1000.0f))


volatile static float target_speed_left = 0, target_speed_right = 0;
volatile static float new_speed_left = 0, new_speed_right = 0;

static float get_speed(unsigned id);
static void set_speed(unsigned id, float output);
static float get_time(void);

#define DEFAULT_SPEED_PID { \
    .compute_interval = MOTOR_CONTROL_INTERVAL_MS,  \
    .proportional_gain = 1.5f,    \
    .integral_gain = 0.3f,        \
    .derivative_gain = 0.0f       \
}
#define DEFAULT_CONTROL_LOOP_PARAMS \
    .target = 0.0f,             \
    .min_output = MIN_SPEED,    \
    .max_output = MAX_SPEED,    \
    .pid = DEFAULT_SPEED_PID,   \
    .get_feedback = get_speed,  \
    .set_output = set_speed,    \
    .get_time = get_time,       \
    .last_feedback = 0.0f,      \
    .last_output = 0.0f,        \
    .last_time_run = 0.0f


static control_loop_t control_loops[2] = {
    {
        .name = "Left",
        .id = 0,
        DEFAULT_CONTROL_LOOP_PARAMS,
    },
    {
        .name = "Right",
        .id = 1,
        DEFAULT_CONTROL_LOOP_PARAMS,
    }
};


extern void control_loop_init(void) {
    debugf("&Left,t,target_speed,current_speed,motor_command\n");
    debugf("&Right,t,target_speed,current_speed,motor_command\n");
}


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


static float get_speed(unsigned id) {
    switch(id) {
    case 0:
        return encoder_get_speed(LEFT_SIDE, WHEEL_RADIUS, SENSOR_TICK_TO_RAD);
        break;
    case 1:
        return encoder_get_speed(RIGHT_SIDE, WHEEL_RADIUS, SENSOR_TICK_TO_RAD);
        break;
    default:
        return 0.0f;
        break;
    }
}


static void set_speed(unsigned id, float output) {
    int32_t output_percent = output * 100.0f / MAX_SPEED;
    actuator_t side; 
    switch(id) {
    case 0:
        side = LEFT_SIDE;
        break;
    case 1:
        side = RIGHT_SIDE;
        break;
    default:
        side = NO_SIDE;
        break;
    }
    if(side == NO_SIDE) { return; }

    motor_set_dir(side, (output > 0)? MOTOR_DIR_FORWARD: MOTOR_DIR_REVERSE);
    motor_set_speed(side, (output > 0)? output_percent: -output_percent);
}


static float get_time(void) {
    return (float)millis() / 1000.0f;
}


extern void do_control_loop(void) {
    //! Clamp target speed with MAX_ACCEL
    if(new_speed_left - target_speed_left > 0.0f) {
        if(new_speed_left - target_speed_left > MAX_ACCEL_PER_LOOP) {
            target_speed_left += MAX_ACCEL_PER_LOOP;
        }
        else {
            target_speed_left = new_speed_left;
        }
    }
    else {
        if(new_speed_left - target_speed_left < -MAX_ACCEL_PER_LOOP) {
            target_speed_left -= MAX_ACCEL_PER_LOOP;
        }
        else {
            target_speed_left = new_speed_left;
        }
    }
    if(new_speed_right - target_speed_right > 0.0f) {
        if(new_speed_right - target_speed_right > MAX_ACCEL_PER_LOOP) {
            target_speed_right += MAX_ACCEL_PER_LOOP;
        }
        else {
            target_speed_right = new_speed_right;
        }
    }
    else {
        if(new_speed_right - target_speed_right < -MAX_ACCEL_PER_LOOP) {
            target_speed_right -= MAX_ACCEL_PER_LOOP;
        }
        else {
            target_speed_right = new_speed_right;
        }
    }

    set_target(&control_loops[0], target_speed_left);
    set_target(&control_loops[1], target_speed_left);

    for(unsigned i = 0; i < 2; i++) {
        control_loop_run(&control_loops[i]);
    }

    static uint8_t debug_cnt = 0;
    if(debug_cnt == 0) {
        debugf("@Left,,%.3f,%.3f,%.3f\n", control_loops[0].target, control_loops[0].last_feedback, control_loops[0].last_output);
        debugf("@Right,,%.3f,%.3f,%.3f\n", control_loops[0].target, control_loops[0].last_feedback, control_loops[0].last_output);
        debug_cnt = 4;
    }
    debug_cnt--;
}


extern void set_target_speed_percent(int32_t target_percent_left, int32_t target_percent_right) {
    //! NOTE: speed = [-MAX_SPEED..MAX_SPEED]
    if(target_percent_left > 100) { target_percent_left = 100; }
    else if(target_percent_left < -100) { target_percent_left = -100; }
    if(target_percent_right > 100) { target_percent_right = 100; }
    else if(target_percent_right < -100) { target_percent_right = -100; }

	new_speed_left = (float)target_percent_left * MAX_SPEED / 100.0f;
	new_speed_right = (float)target_percent_right * MAX_SPEED / 100.0f;
    debugf("@TS,,%0.3f,%0.3f\n", new_speed_left, new_speed_right);
}



extern void update_speed_pid(actuator_t side, float p, float i, float d) {
    if(side == LEFT_SIDE || side == BOTH_SIDE) {
        control_loops[0].pid.proportional_gain = p;
        control_loops[0].pid.integral_gain = i;
        control_loops[0].pid.derivative_gain = d;
        control_loops[0].pid.integral_error = 0;
        debugf("Update Left, p=%f, i=%f, d=%f\n", p, i, d);
    }
    if(side == RIGHT_SIDE || side == BOTH_SIDE) {
        control_loops[1].pid.proportional_gain = p;
        control_loops[1].pid.integral_gain = i;
        control_loops[1].pid.derivative_gain = d;   
        control_loops[1].pid.integral_error = 0;
        debugf("Update Right, p=%f, i=%f, d=%f\n", p, i, d);
    }
}
