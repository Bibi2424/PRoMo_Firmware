#ifndef WS2812B_H
#define WS2812B_H

#include <stdint.h>
#include <stdbool.h>


#define USE_DUMMY_IMPLEMENTATION	0
#define USE_SPI_IMPLEMENTATION		1
#define WS2812B_DRIVE_METHOD		USE_DUMMY_IMPLEMENTATION


typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} rgb_t;



extern void ws2812b_init(void);
extern void ws2812b_send(rgb_t *strip, uint16_t size);


#endif
