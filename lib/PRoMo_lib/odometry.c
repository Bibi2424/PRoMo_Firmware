#include <stdint.h>
#include <stdbool.h>

#include "odometry.h"



static float convert_to_m_per_s(const float tick_speed, const float elapse_time, const float wheel_radius, const float tick_per_wheel_turn_div_pi) {
    //! NOTE: speed_ms = [-MAX_SPEED/s..MAX_SPEED/s]
    float speed_ms = tick_speed / elapse_time * tick_per_wheel_turn_div_pi * wheel_radius ;
    return speed_ms;
}


extern float odometry_get_speed(odometry_t *odo) {
    float speed = 0;
    float now = get_time_microsecond() / 1000000.0f;

    //! Need to compute that diff from int16_t to not get errors, later moving to float for avoiding overflow during calibration
    int16_t speed16 = (int16_t)odo->count - (int16_t)odo->last_count;
    speed = (float)speed16;
    if(odo->invert) { speed = -speed; }
    odo->last_count = odo->count;

    float elapse = (now - odo->last_compute_time);
    // Wrap around condition
    if(now < odo->last_compute_time) { elapse += 1.0f; }
    odo->last_compute_time = now;
    speed = convert_to_m_per_s(speed, elapse, odo->wheel_radius, odo->tick_to_rad);

    return speed;
}
