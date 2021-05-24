#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "main.h"


#define SCHEDULER_MAX_EVENT	20

#define SCHEDULER_TASK_LED1				0
#define SCHEDULER_TASK_LED2				1
#define SCHEDULER_TASK_LOST_CONNECTION	2
#define SCHEDULER_TASK_VL53_GET			3


#define UNUSED 		0x00
#define NEW			0x01
#define USED		0x02

#define SCHEDULER_ONE_SHOT	1
#define SCHEDULER_ALWAYS	-1

#define NOW		0
#define MS		100
#define SECOND  1000*MS
#define MINUTE	60*SECOND


typedef struct {
	uint8_t 	status;
	int16_t 	n;
	uint32_t	time_left;
	uint32_t	period;
	void		(*callback) (void);
}scheduler_event_t;


extern void 	scheduler_init(void);
extern void 	scheduler_add_event(uint8_t id, uint32_t period, int16_t number_of_trigger, void (*callback)(void));
extern void 	scheduler_remove_event(uint8_t id);
extern uint8_t 	scheduler_get_state(uint8_t id);

#endif
