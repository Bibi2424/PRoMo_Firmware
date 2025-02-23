#ifndef TIME_H
#define TIME_H


#include <stdint.h>
#include <stdbool.h>


#define US_TO_MS 	(1000UL)
#define US_TO_S 	(1000 * US_TO_MS)

extern void time_init(void);

extern uint64_t get_time_absolute_us(void);
extern uint32_t get_time_microsecond(void);
extern uint32_t get_time_millisecond(void);
static inline uint32_t millis(void) { return get_time_millisecond(); }


#endif