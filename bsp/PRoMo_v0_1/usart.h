#ifndef USART_H
#define USART_H

#include "bsp.h"

#define RX_BUFFER_SIZE		256


typedef void (*get_char_cb_t)(uint8_t rx_char);

extern void _Error_Handler(char *, int);

extern void MX_USART1_UART_Init(uint32_t baudrate, get_char_cb_t get_char_cb);
extern void MX_USART6_UART_Init(uint32_t baudrate, get_char_cb_t get_char_cb);

#endif
