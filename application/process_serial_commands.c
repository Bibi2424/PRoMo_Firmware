#define DEBUG_THIS_FILE	DEBUG_PROCESS_SERIAL_COMMANDS_FILE

#include <string.h>
#include <stdlib.h>
#include "process_serial_commands.h"
#include "global.h"
#include "usart.h"
#include "motor.h"
#include "nrf24l01.h"

static char commands[RX_BUFFER_SIZE];

static char* get_next_word(char *buffer, uint8_t is_new) {
	if(is_new) 	{ return strtok(buffer, " "); }
	else 		{ return strtok(NULL, " "); }
}


extern uint16_t process_serial_buffer(char* buffer, uint16_t buffer_size) {
	strncpy(commands, buffer, buffer_size);
	// debugf("RX[%u]: %s\r\n", buffer_size, buffer);

	char *word = get_next_word(commands, TRUE);
	while(word) {
		// debugf("W[%X]: %s\n", commands, word);

		if(strcmp(word, "echo") == 0) {
			word = get_next_word(commands, FALSE);
			uint8_t echo = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			set_echo(echo);
		}
		else if(strcmp(word, "set-speed") == 0) {
			word = get_next_word(commands, FALSE);
			uint8_t speed = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			if(speed > 100) { speed = 100; }
			motor_right_set_speed(speed);
			motor_left_set_speed(speed);
		}
		else if(strcmp(word, "nrf-send") == 0) {
			word = get_next_word(commands, FALSE);
			uint8_t data = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			nrf_send(data);
		}
		else if(strcmp(word, "nrf-read") == 0) {
			word = get_next_word(commands, FALSE);
			uint8_t data = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			nrf_read_reg(data);
		}
		else {
			debugf("Unknown command \'%s\'\r\n", word);
		}

		word = get_next_word(commands, FALSE);
	}

	return 0xffff;
}