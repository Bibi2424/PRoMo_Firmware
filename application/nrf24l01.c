#define DEBUG_THIS_FILE	DEBUG_NRF24L01_FILE

#include "nrf24l01.h"
#include "global.h"
#include "gpio.h"

#include "stm32f4xx_ll_spi.h"

void SPI2_NRF24L01_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct;

	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	//! CE - PB2, CSN - PC12
	GPIO_InitStruct.Pin = LL_GPIO_PIN_2 | LL_GPIO_PIN_12;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	CE_LOW;
	CSN_HIGH;

	//! SCK - PB13, MISO - PB14, MOSI - PB15
	GPIO_InitStruct.Pin = LL_GPIO_PIN_13 | LL_GPIO_PIN_14 | LL_GPIO_PIN_15;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	//! Interrupt
	// NVIC_SetPriority(SPI2_IRQn, 0);
	// NVIC_EnableIRQ(SPI2_IRQn);

	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);

	// LL_SPI_SetBaudRatePrescaler(SPI2, LL_SPI_BAUDRATEPRESCALER_DIV32); //1.3MHz
	LL_SPI_SetBaudRatePrescaler(SPI2, LL_SPI_BAUDRATEPRESCALER_DIV64); //666kHz
	LL_SPI_SetTransferDirection(SPI2, LL_SPI_FULL_DUPLEX);
	LL_SPI_SetClockPhase(SPI2, LL_SPI_PHASE_1EDGE);
	LL_SPI_SetClockPolarity(SPI2, LL_SPI_POLARITY_LOW);

	LL_SPI_SetTransferBitOrder(SPI2, LL_SPI_MSB_FIRST);
	LL_SPI_SetDataWidth(SPI2, LL_SPI_DATAWIDTH_8BIT);
	LL_SPI_SetNSSMode(SPI2, LL_SPI_NSS_SOFT);
	LL_SPI_SetMode(SPI2, LL_SPI_MODE_MASTER);

	// LL_SPI_EnableIT_RXNE(SPI2);
	// LL_SPI_EnableIT_TXE(SPI2);
	// LL_SPI_EnableIT_ERR(SPI2);

	LL_SPI_Enable(SPI2);

}


uint8_t nrf_send(uint8_t data) {
	uint8_t status = 0;
	printf("SPI Send: %u\r\n", data);
	LL_SPI_TransmitData8(SPI2, data);
	while(LL_SPI_IsActiveFlag_RXNE(SPI2) == 0 || LL_SPI_IsActiveFlag_BSY(SPI2));
	status = LL_SPI_ReceiveData8(SPI2);
	printf("SPI Status: %u\r\n", status);
	return status;
}


uint8_t nrf_read_reg(uint8_t reg) {
	CSN_LOW;
	nrf_send(0x1f & reg);
	uint8_t value = nrf_send(0xff);
	CSN_HIGH;
	return value;
}



// void SPI1_IRQHandler(void) {
// 	uint8_t data;
// 	/* Check RXNE flag value in ISR register */
// 	if(LL_SPI_IsActiveFlag_RXNE(SPI2)) {
// 		/* Call function Slave Reception Callback */
// 		// SPI1_Rx_Callback();
// 		data = LL_SPI_ReceiveData8(SPI2);
// 		printf("SPI RX: %u\r\n", data);
// 	}
// 	/* Check RXNE flag value in ISR register */
// 	else if(LL_SPI_IsActiveFlag_TXE(SPI2)) {
// 		/* Call function Slave Reception Callback */
// 		// SPI1_Tx_Callback();
// 		printf("SPI TX Done\r\n");
// 	}
// 	/* Check STOP flag value in ISR register */
// 	else if(LL_SPI_IsActiveFlag_OVR(SPI2)) {
// 		/* Call Error function */
// 		// SPI1_TransferError_Callback();
// 		printf("SPI Error\r\n");
// 	}
// }

