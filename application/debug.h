#ifndef DEBUG_H
#define DEBUG_H
#include <stdint.h>
#include "global.h"

/*------------------------- DEFINE -------------------------*/
// #define DBG_USART_BAUDRATE 		115200

#define DEBUG_MAIN_FILE 						1
#define DEBUG_MOTOR_FILE						1
#define DEBUG_ENCODER_FILE						1
#define DEBUG_GPIO_FILE							1
#define DEBUG_PROCESS_SERIAL_COMMANDS_FILE		1
#define DEBUG_SCHEDULER_FILE 					0
#define DEBUG_USART_FILE						1
#define DEBUG_NRF24L01_FILE						1
#define DEBUG_I2C_FILE							1

/*------------------------- MACRO -------------------------*/
//! define DEBUG_THIS_FILE in each files you want to print
// #define debugf(...) 	__asm("nop")
#define debugf(...) 	do { if (DEBUG_THIS_FILE && DEBUG_ENABLE) printf(__VA_ARGS__); } while (0)


#endif
