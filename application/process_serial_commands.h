#ifndef PROCESS_SERIAL_COMMANDS_H
#define PROCESS_SERIAL_COMMANDS_H

#include "main.h"


extern void serial_debug_get_byte(uint8_t rx_char);
extern void set_echo(uint8_t is_echo);
extern uint16_t process_serial_buffer(char* buffer, uint16_t buffer_size);


#endif