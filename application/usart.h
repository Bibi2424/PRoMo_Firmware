#ifndef USART_H
#define USART_H

#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx_ll_rcc.h"
#include "main.h"

#define RX_BUFFER_SIZE		256

extern void set_echo(uint8_t is_echo);

extern void _Error_Handler(char *, int);

extern void MX_USART1_UART_Init(uint32_t baudrate);
extern void MX_USART6_UART_Init(uint32_t baudrate);

#endif
