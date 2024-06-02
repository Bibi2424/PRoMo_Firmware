#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H



extern void control_loop_init(void);
extern void do_control_loop(void);
extern void set_target_speed_percent(int32_t target_left, int32_t target_right);

extern void update_speed_pid(actuator_t side, float p, float i, float d);

#endif
