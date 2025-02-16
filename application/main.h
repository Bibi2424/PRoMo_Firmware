#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "debug.h"
#include "bsp.h"

#define xstr(s) str(s)
#define str(s) #s

//! For motor and encoder
typedef enum {
	NO_SIDE = -1,
	LEFT_SIDE = 0,
	RIGHT_SIDE,
	BOTH_SIDE
} actuator_t;



#define MOTOR_CONTROL_INTERVAL_MS      	5
// #define MPU_INTERVAL_MS      			500
#define MPU_INTERVAL_MS      			0
// #define ALEDS_INTERVAL_MS      			500
#define ALEDS_INTERVAL_MS      			0

#define PI 								(3.141592653f)

//! TODO: Clean up and find a better place for those defines
#define MAX_ACCEL						(10.0f)		// m/s2
#define MIN_SPEED						(0.02f)		// m/s
#define MAX_SPEED						(0.33f)		// m/s

#define WHEEL_RADIUS					(0.032f)	// m
#define SENSOR_TICK_PER_REV				(3578)
#define SENSOR_TICK_TO_RAD				(2.0f*PI/(float)SENSOR_TICK_PER_REV)




#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority, 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority, 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority, 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority, 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority, 0 bit  for subpriority */                                                                 
#endif

//! NOTE Comment/Uncomment the line below to expanse the "assert_param" macro in the HAL drivers code
#define USE_FULL_ASSERT    1U 

extern void blink_led1(void);
extern void blink_led2(void);

extern void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

#endif
