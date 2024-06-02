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
#define DEFAULT_SPEED_PID { \
    .compute_interval = MOTOR_CONTROL_INTERVAL_MS,  \
    .proportional_gain = 1.5f,    \
    .integral_gain = 0.3f,        \
    .derivative_gain = 0.0f       \
}
static pid_controller_t pid_speed_left = DEFAULT_SPEED_PID;
static pid_controller_t pid_speed_right = DEFAULT_SPEED_PID;


extern void control_loop_init(void) {
    debugf("&Left,t,target_speed,current_speed,motor_command\n");
    debugf("&Right,t,target_speed,current_speed,motor_command\n");
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

    //! Encoder
    float current_speed_left = encoder_get_speed(LEFT_SIDE, WHEEL_RADIUS, SENSOR_TICK_TO_RAD);
    float current_speed_right = encoder_get_speed(RIGHT_SIDE, WHEEL_RADIUS, SENSOR_TICK_TO_RAD);

    //! PID
    float motor_command_left = pid_compute(&pid_speed_left, target_speed_left, current_speed_left);
    float motor_command_right = pid_compute(&pid_speed_right, target_speed_right, current_speed_right);

    //! TODO: Look into anti-windup a bit more

    //! Clamp motor command
    if(motor_command_left > MAX_SPEED) { motor_command_left = MAX_SPEED; }
    else if(motor_command_left < -MAX_SPEED) { motor_command_left = -MAX_SPEED; }
    else if(motor_command_left > 0.0f && motor_command_left < MIN_SPEED) { motor_command_left = 0.0f; }
    else if(motor_command_left < 0.0f && motor_command_left > -MIN_SPEED) { motor_command_left = 0.0f; }
    if(motor_command_right > MAX_SPEED) { motor_command_right = MAX_SPEED; }
    else if(motor_command_right < -MAX_SPEED) { motor_command_right = -MAX_SPEED; }
    else if(motor_command_right > 0.0f && motor_command_right < MIN_SPEED) { motor_command_right = 0.0f; }
    else if(motor_command_right < 0.0f && motor_command_right > -MIN_SPEED) { motor_command_right = 0.0f; }

    // Scale command to percent of max_speed
    int32_t motor_percent_left = motor_command_left * 100.0f / MAX_SPEED;
    int32_t motor_percent_right = motor_command_right * 100.0f / MAX_SPEED;

    //! Set Motor commands
    if(motor_percent_left > 0) {
        motor_set_dir(LEFT_SIDE, MOTOR_DIR_FORWARD);
        motor_set_speed(LEFT_SIDE, motor_percent_left);
    } 
    else {
        motor_set_dir(LEFT_SIDE, MOTOR_DIR_REVERSE);
        motor_set_speed(LEFT_SIDE, -motor_percent_left);
    }

    if(motor_percent_right > 0) {
        motor_set_dir(RIGHT_SIDE, MOTOR_DIR_FORWARD);
        motor_set_speed(RIGHT_SIDE, motor_percent_right);
    } 
    else {
        motor_set_dir(RIGHT_SIDE, MOTOR_DIR_REVERSE);
        motor_set_speed(RIGHT_SIDE, -motor_percent_right);
    }

    static uint8_t debug_cnt = 0;
    if(debug_cnt == 0) {
        debugf("@Left,,%.3f,%.3f,%.3f,%.3f\n", target_speed_left, current_speed_left, motor_command_left, pid_speed_left.integral_error);
        debugf("@Right,,%.3f,%.3f,%.3f,%.3f\n", target_speed_right, current_speed_right, motor_command_right, pid_speed_left.integral_error);
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
        pid_speed_left.proportional_gain = p;
        pid_speed_left.integral_gain = i;
        pid_speed_left.derivative_gain = d;
        pid_speed_left.integral_error = 0;
        debugf("Update Left, p=%f, i=%f, d=%f\n", p, i, d);
    }
    if(side == RIGHT_SIDE || side == BOTH_SIDE) {
        pid_speed_right.proportional_gain = p;
        pid_speed_right.integral_gain = i;
        pid_speed_right.derivative_gain = d;   
        pid_speed_right.integral_error = 0;
        debugf("Update Right, p=%f, i=%f, d=%f\n", p, i, d);
    }
}
