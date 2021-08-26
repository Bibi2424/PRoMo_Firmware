#ifndef WS2812B_H
#define WS2812B_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_t;


extern void ws2812b_init(void);
extern void ws2812b_send(rgb_t *strip, uint16_t size);


#endif
