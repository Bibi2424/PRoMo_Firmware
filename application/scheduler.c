#define DEBUG_THIS_FILE		DEBUG_SCHEDULER_FILE

#include <stdio.h>
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_tim.h"
#include "scheduler.h"
#include "global.h"
#include "debug.h"

#define SCHEDULER_TIM 		TIM9
#define SCHEDULER_TIM_IRQn	TIM1_BRK_TIM9_IRQn

/*------------------------- VARIABLES -------------------------*/
volatile scheduler_event_t scheduler[SHEDULER_MAX_EVENT];

/*------------------------- PROTOTYPES -------------------------*/
static void 	scheduler_handler(uint32_t elapse_time);
static void 	update_schedules(uint32_t elapse_time);
static int16_t 	find_next_event(void);

/*------------------------- EXTERN FUNCTIONS -------------------------*/

//! TODO: Change from using TIM9 overflow to CH1
extern void scheduler_init(void) {
	uint8_t i;

  	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM9);

  	//! Init Timer9 for base generation of PWM
	LL_TIM_InitTypeDef LL_TIM_InitStruct;
	LL_TIM_InitStruct.Prescaler = __LL_TIM_CALC_PSC(SystemCoreClock, 10000);
	LL_TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	// LL_TIM_InitStruct.Autoreload = __LL_TIM_CALC_ARR(SystemCoreClock, LL_TIM_InitStruct.Prescaler, 1000);
	LL_TIM_InitStruct.Autoreload = 0x0; //! This will be set by the lib
	LL_TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	LL_TIM_InitStruct.RepetitionCounter = (uint8_t)0x00;
	LL_TIM_Init(SCHEDULER_TIM, &LL_TIM_InitStruct);

	NVIC_SetPriority(TIM1_BRK_TIM9_IRQn, 0);
	NVIC_EnableIRQ(TIM1_BRK_TIM9_IRQn);

	//! NOTE: Do not need to start counter as long as no event is added
  	// LL_TIM_EnableCounter(SCHEDULER_TIM);			//! Enable counter
	
	for(i = 0; i < SHEDULER_MAX_EVENT; i++) {
		scheduler[i].status = UNUSED;
		scheduler[i].n = 0;
		scheduler[i].time_left = 0;
		scheduler[i].period = 0;
		scheduler[i].callback = NULL;
	}
}


extern void scheduler_add_event(uint8_t id, uint32_t period, int16_t number_of_trigger, void (*callback)(void)) {
	debugf("Add event [%u] in %lums for %d times\r\n", id, period, number_of_trigger);
	if(id >= SHEDULER_MAX_EVENT) {return;}
	if(period == 0) {
		scheduler[id].status = UNUSED;
		callback();
		return;
	}
	//! Mark as new to skip the first update_schedules() call
	scheduler[id].status = NEW;
	scheduler[id].n = number_of_trigger;
	scheduler[id].time_left = period*10;
	scheduler[id].period = period*10;
	scheduler[id].callback = callback;
	
	// if
	scheduler_handler(LL_TIM_GetCounter(SCHEDULER_TIM));
}


extern void scheduler_remove_event(uint8_t id) {
	debugf("Remove event [%u]\r\n", id);
	scheduler[id].status = UNUSED;
	scheduler[id].time_left = 0;
	scheduler[id].callback = NULL;
}


extern uint8_t scheduler_get_state(uint8_t id) {
	return scheduler[id].status;
}

/*------------------------- STATIC FUNCTIONS -------------------------*/
void TIM1_BRK_TIM9_IRQHandler(void) {
  /* Check whether update interrupt is pending */
	if(LL_TIM_IsActiveFlag_UPDATE(SCHEDULER_TIM) == 1) {
		scheduler_handler(LL_TIM_GetAutoReload(SCHEDULER_TIM));
		LL_TIM_ClearFlag_UPDATE(SCHEDULER_TIM); 			// Clear the update interrupt flag
	}
}


static void scheduler_handler(uint32_t elapse_time) {
	uint8_t i;
	int16_t next_id;

	// debugf("SCH[%lu]\r\n", elapse_time);
	//! Stop Timer, TimerInt and reset Timer
	LL_TIM_DisableIT_UPDATE(SCHEDULER_TIM);
	LL_TIM_DisableCounter(SCHEDULER_TIM);
	LL_TIM_SetCounter(SCHEDULER_TIM, 0);
	update_schedules(elapse_time);

	for(i = 0; i < SHEDULER_MAX_EVENT; i++) {
		if(scheduler[i].status == USED && scheduler[i].time_left == 0) {
			if(scheduler[i].n != 0) {
				if(scheduler[i].n > 0) {scheduler[i].n--;}
				scheduler[i].time_left = scheduler[i].period;
			}
			else {
				debugf("Event[%u] stoped\r", i);
				scheduler[i].status = UNUSED;
			}
			if(scheduler[i].callback != NULL) {
				// debugf("Callback for event [%d]\r\n", i);
				scheduler[i].callback();
			}
		}
	}

	next_id = find_next_event();
	//! Configure Timer for next event if any
	if(next_id >= 0) {
		debugf("Next event [%d] in %lums\r\n", next_id, scheduler[next_id].time_left/10);
		LL_TIM_SetAutoReload(SCHEDULER_TIM, scheduler[next_id].time_left);
		LL_TIM_EnableCounter(SCHEDULER_TIM);
		LL_TIM_EnableIT_UPDATE(SCHEDULER_TIM);
	}
}


static void update_schedules(uint32_t elapse_time) {
	uint8_t i;

	// debugf("Remove %lums to all events\r\n", elapse_time/10);
	for(i = 0; i < SHEDULER_MAX_EVENT; i++) {
		if(scheduler[i].status == USED) {
			if(elapse_time >= scheduler[i].time_left) {
				scheduler[i].time_left = 0;
			}
			else {
				scheduler[i].time_left -= elapse_time;	
			}
		}
		else if(scheduler[i].status == NEW) {
			scheduler[i].status = USED;
		}
	}
}


static int16_t find_next_event(void) {
	uint8_t i;
	int16_t id = -1;
	uint32_t min_time = 0xFFFFFFFFUL;
	for(i = 0; i < SHEDULER_MAX_EVENT; i++) {
		if(scheduler[i].status == USED && scheduler[i].time_left < min_time) {
			min_time = scheduler[i].time_left;
			id = i;
		}
	}
	return id;
}
