#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "stm32f4xx.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_spi.h"

#include "debug.h"


#define xstr(s) str(s)
#define str(s) #s


#if HW_TYPE == HW_PROMO_V0_1
#include "promo_v0_1_bsp.h"
#endif

//! Debug
#define DEBUG_UART			6
#define DEBUG_BAUDRATE		921600UL
// #define DEBUG_BAUDRATE		230400UL


//! For motor and encoder
typedef enum {
	LEFT_SIDE,
	RIGHT_SIDE,
	BOTH_SIDE
} actuator_t;



#define MOTOR_CONTROL_INTERVAL_MS      	10
// #define MPU_INTERVAL_MS      			500
#define MPU_INTERVAL_MS      			0
// #define ALEDS_INTERVAL_MS      			500
#define ALEDS_INTERVAL_MS      			0


//! TODO: Clean up and find a better place for those defines
#define MAX_SPEED						180
#define WHEEL_RADIUS_MM					32L
#define TICKS_PER_WHEEL_TURN_DIV_PI		1138L	/* 3576 / PI */




#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority, 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority, 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority, 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority, 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority, 0 bit  for subpriority */                                                                 
#endif

//! NOTE Comment/Uncomment the line below to expanse the "assert_param" macro in the HAL drivers code
#define USE_FULL_ASSERT    1U 

extern uint32_t millis(void);
extern void blink_led1(void);
extern void blink_led2(void);

extern void UserButton_Callback(void);
extern void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

#endif
