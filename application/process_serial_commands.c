#define DEBUG_THIS_FILE	DEBUG_PROCESS_SERIAL_COMMANDS_FILE

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"
#include "debug.h"

#include "process_serial_commands.h"
#include "main.h"
#include "time.h"
#include "gpio.h"
#include "usart.h"
#include "scheduler.h"
#include "drive_speed_control.h"
#include "motor.h"
#include "encoder.h"
#include "spi.h"
#include "nrf24l01.h"
#include "i2c.h"
#include "sensors.h"

static char commands[RX_BUFFER_SIZE];

static char* get_next_word(char *buffer, uint8_t is_new) {
	if(is_new) 	{ return strtok(buffer, " "); }
	else 		{ return strtok(NULL, " "); }
}


extern uint16_t process_serial_buffer(char* buffer, uint16_t buffer_size) {
	uint16_t i;
	strncpy(commands, buffer, buffer_size);
	// debugf("RX[%u]: %s\r\n", buffer_size, buffer);

	char *word = get_next_word(commands, true);
	while(word) {
		// debugf("W[%X]: %s\n", commands, word);

		if(strcmp(word, "echo") == 0) {
			word = get_next_word(commands, false);
			uint8_t echo = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			set_echo(echo);
		}
		else if(strcmp(word, "wait") == 0) {
			word = get_next_word(commands, false);
			uint32_t time_ms = (uint32_t)(strtoul(word, NULL, 0));
			LL_mDelay(time_ms);
		}
		else if(strcmp(word, "reset") == 0) {
			NVIC_SystemReset();
		}
		else if(strcmp(word, "millis") == 0) {
			printf("%lu\n", millis());
		}
		else if(strcmp(word, "say") == 0) {
			word = get_next_word(commands, false);
			printf("Say: %s\n", word);
		}

		//! Scheduler
		else if(strcmp(word, "scheduler") == 0) {
			debug_scheduler();
		}

		//! VL53L0X
		else if(strcmp(word, "vl53.get") == 0) {
			word = get_next_word(commands, false);
			uint8_t id = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			statInfo_t range;
			uint16_t distance = sensors_vl53l0x_range_one(id, &range);
			printf("%u - %u\n", id, distance);
		}
		else if(strcmp(word, "vl53.get-all") == 0) {
			statInfo_t ranges[4];
			sensors_vl53l0x_range_all(ranges);
			printf("L:%u F:%u R:%u B:%u\n", ranges[0].rawDistance, ranges[1].rawDistance, ranges[2].rawDistance, ranges[3].rawDistance);
		}

		//! I2C
		else if(strcmp(word, "i2c.write") == 0) {
			word = get_next_word(commands, false);
			uint8_t address = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			word = get_next_word(commands, false);
			uint8_t mem = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			word = get_next_word(commands, false);
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
			word = get_next_word(commands, false);
			uint8_t address = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			word = get_next_word(commands, false);
			uint8_t mem = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			word = get_next_word(commands, false);
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

		//! CONTROL LOOP
		else if(strcmp(word, "target.speed") == 0) {
			word = get_next_word(commands, false);
			int32_t speed_percent_left = (int32_t)(strtol(word, NULL, 0));
			word = get_next_word(commands, false);
			int32_t speed_percent_right = (int32_t)(strtol(word, NULL, 0));
			set_target_speed_percent(speed_percent_left, speed_percent_right);
		}

		else if(strcmp(word, "pid.set") == 0) {
			char side[10];
			word = get_next_word(commands, false);
			strncpy(side, word, 10);

			word = get_next_word(commands, false);
			float p = (strtoul(word, NULL, 0))/100.0f;
			word = get_next_word(commands, false);
			float i = (strtoul(word, NULL, 0))/100.0f;
			word = get_next_word(commands, false);
			float d = (strtoul(word, NULL, 0))/100.0f;

			if(strcmp(side, "left") == 0) {
				update_speed_pid(LEFT_SIDE, p, i, d);
			}
			else if(strcmp(side, "right") == 0) {
				update_speed_pid(RIGHT_SIDE, p, i, d);
			}
			else if(strcmp(side, "both") == 0) {
				update_speed_pid(BOTH_SIDE, p, i, d);
			}
		}

		//! ENCODER
		else if(strcmp(word, "get-tick") == 0) {
			printf("L=%lu, R=%lu\n", encoder_get_tick_count(LEFT_SIDE), encoder_get_tick_count(RIGHT_SIDE));
		}

		//! MOTOR CONTROL COMMANDS
		else if(strcmp(word, "motor.speed") == 0) {
			char wheels[10];
			word = get_next_word(commands, false);
			strncpy(wheels, word, 10);
			word = get_next_word(commands, false);
			uint32_t speed_percent = (uint32_t)(strtoul(word, NULL, 0) & 0xff);
			if(speed_percent > 100) { speed_percent = 100; }
			
			if(strcmp(wheels, "left") == 0) {
				motor_set_speed(LEFT_SIDE, speed_percent);
			}
			else if(strcmp(wheels, "right") == 0) {
				motor_set_speed(RIGHT_SIDE, speed_percent);
			}
			else if(strcmp(wheels, "both") == 0) {
				motor_set_speed(BOTH_SIDE, speed_percent);
			}
		}
		else if(strcmp(word, "motor.dir") == 0) {
			char wheels[10];
			word = get_next_word(commands, false);
			strncpy(wheels, word, 10);
			word = get_next_word(commands, false);
			uint8_t dir = (uint8_t)(strtoul(word, NULL, 0) & 0xff);

			if(strcmp(wheels, "left") == 0) {
				motor_set_dir(LEFT_SIDE, dir);
			}
			else if(strcmp(wheels, "right") == 0) {
				motor_set_dir(RIGHT_SIDE, dir);
			}
			else if(strcmp(wheels, "both") == 0) {
				motor_set_dir(LEFT_SIDE, dir);
				motor_set_dir(RIGHT_SIDE, dir);
			}
		}

		//! SPI COMMANDS
		else if(strcmp(word, "spi-send") == 0) {
			word = get_next_word(commands, false);
			uint8_t size = (uint8_t)(strtoul(word, NULL, 0) & 0xff);
			uint8_t spi_write_data[10];
			uint8_t spi_read_data[10];
			for(uint8_t i = 0; i < size; i++) {
				word = get_next_word(commands, false);
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
			word = get_next_word(commands, false);
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
			word = get_next_word(commands, false);
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
			word = get_next_word(commands, false);
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

		word = get_next_word(commands, false);
	}

	return 0xffff;
}