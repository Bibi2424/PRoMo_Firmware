#define DEBUG_THIS_FILE DEBUG_DRIVE_SPEED_CONTROL_FILE

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "utils.h"
#include "main.h"
#include "control_loop.h"
#include "encoder.h"
#include "motor.h"


#define MAX_ACCEL_PER_LOOP (MAX_ACCEL * (MOTOR_CONTROL_INTERVAL_MS / 1000.0f))


static float get_speed(unsigned id);
static void set_speed(unsigned id, float output);


#define DEFAULT_SPEED_PID { \
    .compute_interval = MOTOR_CONTROL_INTERVAL_MS,  \
    .proportional_gain = 1.5f,    \
    .integral_gain = 0.3f,        \
    .derivative_gain = 0.0f       \
}
#define DEFAULT_CONTROL_LOOP_PARAMS \
    .target = 0.0f,             \
    .max_input_derivative = MAX_ACCEL_PER_LOOP, \
    .min_output = MIN_SPEED,    \
    .max_output = MAX_SPEED,    \
    .pid = DEFAULT_SPEED_PID,   \
    .get_feedback = get_speed,  \
    .set_output = set_speed,    \
    .new_target = 0.0f,      	\
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


extern void drive_speed_control_init(void) {
    debugf("&Left,t,target_speed,current_speed,motor_command\n");
    debugf("&Right,t,target_speed,current_speed,motor_command\n");
}



volatile static float target_speed_left = 0, target_speed_right = 0;

extern void drive_speed_control_loop(void) {
    set_target(&control_loops[0], target_speed_left);
    set_target(&control_loops[1], target_speed_right);

    for(unsigned i = 0; i < 2; i++) {
        control_loop_run(&control_loops[i], (float)millis() / 1000.0f);
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

	target_speed_left = (float)target_percent_left * MAX_SPEED / 100.0f;
	target_speed_right = (float)target_percent_right * MAX_SPEED / 100.0f;
    debugf("@TS,,%0.3f,%0.3f\n", target_speed_left, target_speed_right);
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
