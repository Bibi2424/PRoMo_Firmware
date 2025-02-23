#ifndef ODOMETRY_H
#define ODOMETRY_H


typedef struct {
	bool invert;
	float wheel_radius;
	float tick_to_rad;

	uint16_t count;
	uint16_t last_count;
	float last_compute_time;
} odometry_t;


extern float odometry_get_speed(odometry_t *odo);


#endif
