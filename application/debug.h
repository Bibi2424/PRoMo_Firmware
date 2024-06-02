#ifndef DEBUG_H
#define DEBUG_H


#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>


/*------------------------- DEFINE -------------------------*/
// #define DBG_USART_BAUDRATE 		115200

#define DEBUG_MAIN_FILE 						1
#define DEBUG_PROCESS_SERIAL_COMMANDS_FILE		1
#define DEBUG_USART_FILE						0
#define DEBUG_GPIO_FILE							0

#define DEBUG_SCHEDULER_FILE 					0

#define DEBUG_CONTROL_LOOP_FILE					1
#define DEBUG_PID_FILE							1
#define DEBUG_MOTOR_FILE						1
#define DEBUG_ENCODER_FILE						0
#define DEBUG_LERP_FILE							1

#define DEBUG_SENSORS_FILE						0
#define DEBUG_I2C_FILE							0

#define DEBUG_RADIO_FILE						0
#define DEBUG_NRF24L01_FILE						0
#define DEBUG_SPI_FILE							0

/*------------------------- MACRO -------------------------*/
//! define DEBUG_THIS_FILE in each files you want to print
// #define debugf(...) 	__asm("nop")
#define debugf(...) 	do { if (DEBUG_THIS_FILE && DEBUG_ENABLE) printf(__VA_ARGS__); } while (0)
#define debug_array(...) 	do { if (DEBUG_THIS_FILE && DEBUG_ENABLE) printf(__VA_ARGS__); } while (0)


static inline void print_array(char *prefix, uint8_t *array, uint8_t size, bool lf) {
	if(prefix != 0) { printf("%s:[", prefix); }
	else { printf("["); }
	for(uint8_t i = 0; i < size; i++) {
		printf("%02X:", array[i]);
	}
	if(lf) { printf("]\n"); }
	else { printf("]"); }
}


#endif
