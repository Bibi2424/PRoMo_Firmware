#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"


enum {
	GPIO_LOW = 0,
	GPIO_HIGH = 1,
};

#define SET_PIN(PORT, PIN, VALUE)	do { if(VALUE == GPIO_LOW) { LL_GPIO_ResetOutputPin(PORT, PIN); } else { LL_GPIO_SetOutputPin(PORT, PIN); } } while(0)
#define TOGGLE_PIN(PORT, PIN)		LL_GPIO_TogglePin(PORT, PIN)


typedef enum {
	GPIO_INTERRUPT_RISING,
	GPIO_INTERRUPT_FALLING
} gpio_interrupt_type_t;

typedef void (*gpio_interrupt_cb)(gpio_interrupt_type_t);

void MX_GPIO_Init(void);
void gpio__register_callback(gpio_interrupt_cb cb);

#endif