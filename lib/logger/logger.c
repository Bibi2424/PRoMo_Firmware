#include "logger.h"

#ifndef MIN
#define MIN(a, b) (((a) < (b))? (a): (b))
#endif


static char logger_buffer[2][LOGGER_MAX_LINE_CHAR];
static char *current_buffer;
static send_log_t send_log_cb = NULL;

extern void logger_init(send_log_t send_log) {
	send_log_cb = send_log;
}


extern bool logger_log(const char* fmt, ...) {
	va_list args;
    va_start(args, fmt);

    if((char*)current_buffer == (char*)&logger_buffer[0][0]) { current_buffer = &logger_buffer[1][0]; }
    else { current_buffer = &logger_buffer[0][0]; }
	int length = vsnprintf(current_buffer, LOGGER_MAX_LINE_CHAR, fmt, args);

	va_end(args);

	if(length < 0) { return false; }
	if(send_log_cb != NULL) { send_log_cb(current_buffer, MIN(LOGGER_MAX_LINE_CHAR, length)); }

	return true;
}
