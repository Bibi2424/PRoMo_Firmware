#define DEBUG_THIS_FILE DEBUG_CONTROL_LOOP_FILE

#include <stdint.h>
#include <stdbool.h>

#include "global.h"
#include "main.h"

#include "control_loop.h"
#include "pid_controller.h"
#include "encoder.h"
#include "motor.h"


volatile static int32_t target_speed_left = 0, target_speed_right = 0;
static pid_controller_t pid_speed_left = {
    .compute_interval = MOTOR_CONTROL_INTERVAL_MS,
    .proportional_gain = 75,
    .integral_gain = 25,
    .derivative_gain = 5,
    .max_integral_error = 1000,
    .min_output = 20,
    .max_output = 250
};
static pid_controller_t pid_speed_right = {
    .compute_interval = MOTOR_CONTROL_INTERVAL_MS,
    .proportional_gain = 75,
    .integral_gain = 25,
    .derivative_gain = 5,
    .max_integral_error = 1000,
    .min_output = 20,
    .max_output = 250
};



extern void do_control_loop(void) {

    //! Encoder
    int32_t current_left_speed = encoder_get_speed(LEFT_SIDE);
    int32_t current_right_speed = encoder_get_speed(RIGHT_SIDE);

    //! PID
    int32_t motor_left_command = pid_compute(&pid_speed_left, target_speed_left, current_left_speed);
    int32_t motor_right_command = pid_compute(&pid_speed_right, target_speed_right, current_right_speed);

    debugf("Left: %ld - %ld - %ld\n", target_speed_left, current_left_speed, motor_left_command);
    debugf("Right: %ld - %ld - %ld\n", target_speed_right, current_right_speed, motor_right_command);

    //! Motors
    if(motor_left_command > 0) {
        motor_set_dir(LEFT_SIDE, MOTOR_DIR_FORWARD);
        motor_set_speed(LEFT_SIDE, (uint32_t)motor_left_command * 100 / MAX_SPEED);
    } 
    else {
        motor_set_dir(LEFT_SIDE, MOTOR_DIR_REVERSE);
        motor_set_speed(LEFT_SIDE, (uint32_t)-motor_left_command * 100 / MAX_SPEED);
    }
    if(motor_right_command > 0) {
        motor_set_dir(RIGHT_SIDE, MOTOR_DIR_FORWARD);
        motor_set_speed(RIGHT_SIDE, (uint32_t)motor_right_command * 100 / MAX_SPEED);
    } 
    else {
        motor_set_dir(RIGHT_SIDE, MOTOR_DIR_REVERSE);
        motor_set_speed(RIGHT_SIDE, (uint32_t)-motor_right_command * 100 / MAX_SPEED);
    }

}


extern void set_target_speed(int32_t left_percent, int32_t right_percent) {
	target_speed_left = left_percent * MAX_SPEED / 100;
	target_speed_right = right_percent * MAX_SPEED / 100;
}



extern void update_speed_pid(actuator_t side, uint32_t p, uint32_t i, uint32_t d) {
    if(side == LEFT_SIDE) {
        pid_speed_left.proportional_gain = p;
        pid_speed_left.integral_gain = i;
        pid_speed_left.derivative_gain = d;
        pid_speed_left.integral_error = 0;
        debugf("Update Left=");
    }
    else if(side == RIGHT_SIDE) {
        pid_speed_right.proportional_gain = p;
        pid_speed_right.integral_gain = i;
        pid_speed_right.derivative_gain = d;   
        pid_speed_right.integral_error = 0;
        debugf("Update Right=");
    }
    debugf("p=%lu, i=%lu, d=%lu\n", p, i, d);
}