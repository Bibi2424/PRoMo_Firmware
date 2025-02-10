#ifndef TIME_H
#define TIME_H


#include <stdint.h>
#include <stdbool.h>


#define MILLIS 	1000UL
#define SECONDS 1000 * 1000UL

extern void time_init(void);

extern uint64_t get_time_absolute_us(void);
extern uint32_t get_time_microsecond(void);
extern uint32_t get_time_millisecond(void);
static inline uint32_t millis(void) { return get_time_millisecond(); }


#endif