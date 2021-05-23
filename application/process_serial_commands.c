#define DEBUG_THIS_FILE	DEBUG_PROCESS_SERIAL_COMMANDS_FILE

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "global.h"
#include "debug.h"

#include "process_serial_commands.h"
#include "main.h"
#include "gpio.h"
#include "usart.h"
#include "motor.h"
#include "spi.h"
#include "nrf24l01.h"
#include "i2c.h"

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
		else if(strcmp(word, "wait") == 0) {
			word = get_next_word(commands, FALSE);
			uint32_t time_ms = (uint32_t)(strtoul(word, NULL, 0));
			LL_mDelay(time_ms);
		}
		else if(strcmp(word, "reset") == 0) {
			NVIC_SystemReset();
		}

		//! I2C
		else if(strcmp(word, "i2c.write") == 0) {
			word = get_next_word(commands, FALSE);
			uint8_t address = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			word = get_next_word(commands, FALSE);
			uint8_t mem = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			word = get_next_word(commands, FALSE);
			uint8_t size = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			if(size > 10) { size = 10; }

			uint8_t i;
			uint8_t buffer[10] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09};
			uint8_t res = i2c1_full_write(address, mem, buffer, size);
			if(res) {
				printf("Error [%u]\n", res);
			}
			else {
				printf("Buffer: [");
				for(i = 0; i < size; i++) { printf("%02X:", buffer[i]); }
				printf("\b]\n");
			}
		}
		else if(strcmp(word, "i2c.read") == 0) {
			word = get_next_word(commands, FALSE);
			uint8_t address = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			word = get_next_word(commands, FALSE);
			uint8_t mem = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			word = get_next_word(commands, FALSE);
			uint8_t size = (uint8_t)(strtoul(word, NULL, 0) & 0xff);

			uint8_t i;
			uint8_t buffer[20];
			uint8_t res = i2c_full_read(I2C1, address, mem, buffer, size);
			if(res) {
				printf("Error [%u]\n", res);
			}
			else {
				printf("Buffer: [");
				for(i = 0; i < size; i++) { printf("%02X:", buffer[i]); }
				printf("\b]\n");
			}
		}

		//! MOTOR CONTROL COMMANDS
		else if(strcmp(word, "set-speed") == 0) {
			char wheels[10];
			word = get_next_word(commands, FALSE);
			strncpy(wheels, word, 10);
			word = get_next_word(commands, FALSE);
			uint8_t speed = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			if(speed > 100) { speed = 100; }
			
			if(strcmp(wheels, "left") == 0) {
				motor_left_set_speed(speed);
			}
			else if(strcmp(wheels, "right") == 0) {
				motor_right_set_speed(speed);
			}
			else if(strcmp(wheels, "both") == 0) {
				motor_right_set_speed(speed);
				motor_left_set_speed(speed);
			}
		}
		else if(strcmp(word, "set-dir") == 0) {
			char wheels[10];
			word = get_next_word(commands, FALSE);
			strncpy(wheels, word, 10);
			word = get_next_word(commands, FALSE);
			uint8_t dir = (uint8_t)(strtoul(word, NULL, 0) & 0xff);

			if(strcmp(wheels, "left") == 0) {
				motor_left_set_dir(dir);
			}
			else if(strcmp(wheels, "right") == 0) {
				motor_right_set_dir(dir);
			}
			else if(strcmp(wheels, "both") == 0) {
				motor_right_set_dir(dir);
				motor_left_set_dir(dir);
			}
		}

		//! SPI COMMANDS
		else if(strcmp(word, "spi-send") == 0) {
			word = get_next_word(commands, FALSE);
			uint8_t size = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			uint8_t spi_write_data[10];
			uint8_t spi_read_data[10];
			for(uint8_t i = 0; i < size; i++) {
				word = get_next_word(commands, FALSE);
				spi_write_data[i] = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			}
			CSN_LOW;
			spi2_send_multiple_bytes_waiting(spi_write_data, spi_read_data, size);
			printf("SPI-READ:[");
			for(uint8_t i = 0; i < size; i++) { printf("%02X:", spi_read_data[i]); }
			printf("]\r\n");
			CSN_HIGH;
		}

		//! NRF COMMANDS
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

		//! LEDS
		else if(strcmp(word, "set-led") == 0) {
			word = get_next_word(commands, FALSE);
			uint8_t led_id = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			GPIO_TypeDef *led_port;
			uint32_t led_pin;
			if(led_id == 1) {
				led_port = LD1_GPIO_Port;
				led_pin = LD1_Pin;
			}
			else if(led_id == 2) {
				led_port = LD2_GPIO_Port;
				led_pin = LD2_Pin;
			}
			else {
				return 0;
			}
			word = get_next_word(commands, FALSE);
			if(strcmp(word, "off") == 0) {
				LL_GPIO_SetOutputPin(led_port, led_pin);
			}
			else if(strcmp(word, "on") == 0) {
				LL_GPIO_ResetOutputPin(led_port, led_pin);
			}
			else if(strcmp(word, "toggle") == 0) {
				LL_GPIO_TogglePin(led_port, led_pin);
			}
		}

		else {
			debugf("Unknown command \'%s\'\r\n", word);
		}

		word = get_next_word(commands, FALSE);
	}

	return 0xffff;
}