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
	uint16_t i;
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
		//! MOTOR CONTROL COMMANDS
		else if(strcmp(word, "set-speed") == 0) {
			word = get_next_word(commands, FALSE);
			uint8_t speed = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			if(speed > 100) { speed = 100; }
			motor_right_set_speed(speed);
			motor_left_set_speed(speed);
		}
		//! NRF COMMANDSa
		// else if(strcmp(word, "nrf-send") == 0) {
		// 	word = get_next_word(commands, FALSE);
		// 	uint8_t data = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
		// 	nrf_send_byte_waiting(data);
		// }
		else if(strcmp(word, "nrf-status") == 0) {
			uint8_t status = nrf_get_status();
			printf("NRF status: 0x%02X\r\n", status);
		}
		else if(strcmp(word, "nrf-get-reg") == 0) {
			word = get_next_word(commands, FALSE);
			uint8_t data = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			uint8_t reg = nrf_read_register(data);
			printf("NRF reg[%u]: 0x%02X\r\n", data, reg);
		}
		else if(strcmp(word, "nrf-test-read") == 0) {
			// uint8_t test[5] = {0xAA, 0x55, 0xAA, 0x55, 0xAA};
			// nrf_write_multiple_bytes_register(NRF24L01_REGISTER_RX_ADDR_P0, test, 5);
			uint8_t ret[5] = {0};
			nrf_read_multiple_bytes_register(NRF24L01_REGISTER_RX_ADDR_P0, ret, 5);
			printf("R:[");for(i=0;i<5;i++){printf("%02X ", ret[i]);}printf("]\r\n");
		}
		else if(strcmp(word, "nrf-test-write") == 0) {
			uint8_t test[5] = {0xAA, 0x55, 0xAA, 0x55, 0xAA};
			nrf_write_multiple_bytes_register(NRF24L01_REGISTER_RX_ADDR_P0, test, 5);
		}
		else if(strcmp(word, "nrf-test-clear") == 0) {
			uint8_t test[5] = {0xAA, 0x55, 0xAA, 0x55, 0xAA};
			nrf_write_multiple_bytes_register(NRF24L01_REGISTER_RX_ADDR_P0, test, 5);
			uint8_t ret[5] = {0};
			nrf_read_multiple_bytes_register(NRF24L01_REGISTER_RX_ADDR_P0, ret, 5);
			printf("R:[");for(i=0;i<5;i++){printf("%02X ", ret[i]);}printf("]\r\n");
		}
		else {
			debugf("Unknown command \'%s\'\r\n", word);
		}

		word = get_next_word(commands, FALSE);
	}

	return 0xffff;
}