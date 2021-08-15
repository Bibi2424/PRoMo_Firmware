#ifndef CONTROL_LOOP_H
#define CONTROL_LOOP_H



extern void do_control_loop(void);
extern void set_target_speed(int32_t left, int32_t right);

extern void update_speed_pid(actuator_t side, uint32_t p, uint32_t i, uint32_t d);

#endif
