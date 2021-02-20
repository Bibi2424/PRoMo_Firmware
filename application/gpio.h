#ifndef GPIO_H
#define GPIO_H

#include "main.h"

#define SET_PIN(PORT, PIN, VALUE)	do { if(VALUE == 0) { LL_GPIO_ResetOutputPin(PORT, PIN); } else { LL_GPIO_SetOutputPin(PORT, PIN); } } while(0)
#define TOGGLE_PIN(PORT, PIN)		LL_GPIO_TogglePin(PORT, PIN)


void MX_GPIO_Init(void);

#endif