#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "logger.h"

//! Debug
#ifndef DEBUG_ENABLE
#define DEBUG_ENABLE 		1
#endif
#ifndef DEBUG_BAUDRATE
#define DEBUG_BAUDRATE		921600UL
// #define DEBUG_BAUDRATE		230400UL
#endif

/*------------------------- DEFINE -------------------------*/
#define DEBUG_MAIN_FILE 						1
#define DEBUG_PROCESS_SERIAL_COMMANDS_FILE		1

#define DEBUG_SCHEDULER_FILE 					0

#define DEBUG_DRIVE_SPEED_CONTROL_FILE			1
#define DEBUG_PID_FILE							1

#define DEBUG_SENSORS_FILE						0

#define DEBUG_RADIO_FILE						0

/*------------------------- MACRO -------------------------*/
//! define DEBUG_THIS_FILE in each files you want to print
// #define debugf(...) 	__asm("nop")
#define debugf(...) 	do { if (DEBUG_THIS_FILE && DEBUG_ENABLE) logger_log(__VA_ARGS__); } while (0)
#define debug_array(...) 	do { if (DEBUG_THIS_FILE && DEBUG_ENABLE) logger_log(__VA_ARGS__); } while (0)

#endif
