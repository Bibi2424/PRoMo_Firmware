#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "utils.h"

#include "usart.h"
#include "gpio.h"


static get_char_cb_t usart1_get_char_cb = NULL;
static get_char_cb_t usart6_get_char_cb = NULL;


//! TODO: In the future, use USART with DMA and either circular buffer or double buffering
extern void MX_USART1_UART_Init(uint32_t baudrate, get_char_cb_t get_char_cb) {
	LL_USART_InitTypeDef USART_InitStruct;
	LL_GPIO_InitTypeDef GPIO_InitStruct;

	/* Peripheral clock enable */
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART6);
	
	//! PA9   ------> USART1_TX
	//! PA10  ------> USART1_RX
	//! PA11  ------> USART1_CTS
	//! PA12  ------> USART1_RTS
	LL_GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.Pin = USART1_TX_Pin | USART1_RX_Pin;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
	LL_GPIO_Init(USART6_GPIO_Port, &GPIO_InitStruct);

	USART_InitStruct.BaudRate = baudrate;
	USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
	USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
	USART_InitStruct.Parity = LL_USART_PARITY_NONE;
	USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
	USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
	LL_USART_Init(USART1, &USART_InitStruct);
	LL_USART_ConfigAsyncMode(USART1);

	NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 6, 0));  
	NVIC_EnableIRQ(USART1_IRQn);
	LL_USART_ClearFlag_ORE(USART1);
	LL_USART_EnableIT_RXNE(USART1);

	usart1_get_char_cb = get_char_cb;
	
	LL_USART_Enable(USART1);
}


extern void MX_USART6_UART_Init(uint32_t baudrate, get_char_cb_t get_char_cb) {
	LL_USART_InitTypeDef USART_InitStruct;

	LL_GPIO_InitTypeDef GPIO_InitStruct;
	/* Peripheral clock enable */
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART6);
	
	//! PC6   ------> USART6_TX
	//! PC7   ------> USART6_RX
	GPIO_InitStruct.Pin = USART6_TX_Pin | USART6_RX_Pin;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_8;
	LL_GPIO_Init(USART6_GPIO_Port, &GPIO_InitStruct);

	USART_InitStruct.BaudRate = baudrate;
	USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
	USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
	USART_InitStruct.Parity = LL_USART_PARITY_NONE;
	USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
	USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
	LL_USART_Init(USART6, &USART_InitStruct);
	LL_USART_ConfigAsyncMode(USART6);

	NVIC_SetPriority(USART6_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 6, 0)); 

	NVIC_EnableIRQ(USART6_IRQn);
	LL_USART_ClearFlag_ORE(USART6);
	LL_USART_EnableIT_RXNE(USART6);

	usart6_get_char_cb = get_char_cb;
	
	LL_USART_Enable(USART6);
}


void USART1_IRQHandler(void) {
	if(LL_USART_IsActiveFlag_RXNE(USART1) && LL_USART_IsEnabledIT_RXNE(USART1)) {
		uint8_t rx_char = LL_USART_ReceiveData8(USART1);
		if(usart1_get_char_cb != NULL) { usart1_get_char_cb(rx_char); }
	}
}


void USART6_IRQHandler(void) {
	/* Check RXNE flag value in SR register */
	if(LL_USART_IsActiveFlag_RXNE(USART6) && LL_USART_IsEnabledIT_RXNE(USART6)) {
		/* RXNE flag will be cleared by reading of DR register (done in call) */
		/* Call function in charge of handling Character reception */
		uint8_t rx_char = LL_USART_ReceiveData8(USART6);
		if(usart6_get_char_cb != NULL) { usart6_get_char_cb(rx_char); }
	}
}
