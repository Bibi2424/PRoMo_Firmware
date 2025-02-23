#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>


#define LOGGER_MAX_LINE_CHAR	256

typedef bool (*send_log_t)(const char* log, const size_t n);

extern void logger_init(send_log_t send_log);
extern bool logger_log(const char* fmt, ...);

#endif
