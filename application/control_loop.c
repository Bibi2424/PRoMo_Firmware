#define DEBUG_THIS_FILE DEBUG_CONTROL_LOOP_FILE

#include <stdint.h>
#include <stdbool.h>

#include "global.h"
#include "debug.h"

#include "control_loop.h"
#include "pid_controller.h"
#include "encoder.h"
#include "motor.h"


volatile static int8_t target_speed_left = 0, target_speed_right = 0;
static pid_controller_t pid_speed_left = {
    .compute_interval = 50,
    .proportional_gain = 1,
    .integral_gain = 0,
    .derivative_gain = 0
};
static pid_controller_t pid_speed_right = {
    .compute_interval = 50,
    .proportional_gain = 1,
    .integral_gain = 0,
    .derivative_gain = 0
};



extern void do_control_loop(void) {

    // debugf("TARGET: %d - %d\n", target_speed_left, target_speed_right);

    //! Encoder
    int32_t current_left_speed = encoder_left_get_speed();
    int32_t current_right_speed = encoder_right_get_speed();
    // debugf("ENCODER: %ld - %ld\n", current_left_speed, -current_right_speed);

    //! PID
    int32_t motor_left_command = target_speed_left;
    int32_t motor_right_command = target_speed_right;
    // int32_t motor_left_command = pid_compute(&pid_speed_left, target_speed_left, current_left_speed);
    // int32_t motor_right_command = pid_compute(&pid_speed_right, target_speed_right, current_right_speed);
    // debugf("PID OUT: %ld - %ld\n", motor_left_command, motor_right_command);

    // debugf("Left: %d - %ld - %ld\n", target_speed_left, current_left_speed, motor_left_command);
    // debugf("Right: %d - %ld - %ld\n", target_speed_right, current_right_speed, motor_right_command);

    //! Motors
    if(motor_left_command > 0) {
        motor_left_set_dir(1);
        motor_left_set_speed((uint32_t)motor_left_command);
    } 
    else {
        motor_left_set_dir(0);
        motor_left_set_speed((uint32_t)-motor_left_command);
    }
    if(motor_right_command > 0) {
        motor_right_set_dir(1);
        motor_right_set_speed((uint32_t)motor_right_command);
    } 
    else {
        motor_right_set_dir(0);
        motor_right_set_speed((uint32_t)-motor_right_command);
    }


}


extern void set_target_speed(int32_t left, int32_t right) {
	target_speed_left = left;
	target_speed_right = right;	
}