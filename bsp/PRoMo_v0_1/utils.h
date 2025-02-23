#ifndef UTILS_H
#define UTILS_H


#include <stdint.h>
#include <stdbool.h>


#define MILLIS 	1000UL
#define SECONDS 1000 * 1000UL

#define ABS(x) 	( ((x) < 0) ? -(x) : (x) )

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))

#endif