#ifndef USART_H
#define USART_H

#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_rcc.h"
#include "main.h"

#define USART6_BAUDRATE	921600
#define RX_BUFFER_SIZE	100

extern void set_echo(uint8_t is_echo);

extern void _Error_Handler(char *, int);

void MX_USART6_UART_Init(void);

#endif
