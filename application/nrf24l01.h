#ifndef NRF24L01_H
#define NRF24L01_H

#include "main.h"

#define CE_LOW		LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_2)
#define CE_HIGH		LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_2);
#define CSN_LOW		LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_12);
#define CSN_HIGH	LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_12);


void SPI2_NRF24L01_Init(void);
uint8_t nrf_send(uint8_t data);
uint8_t nrf_read_reg(uint8_t reg);


#endif