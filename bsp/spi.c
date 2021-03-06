#define DEBUG_THIS_FILE   DEBUG_SPI_FILE

#include "global.h"
#include "debug.h"

#include "spi.h"
#include "gpio.h"

#define SPI_NOP		0xFF


extern void spi_init(SPI_TypeDef *SPIx) {
	uint32_t clock_periph;
	IRQn_Type spi_irq_type;

	if(SPIx == SPI1) {
		clock_periph = LL_APB2_GRP1_PERIPH_SPI1;
		spi_irq_type = SPI1_IRQn;
	}
	else if(SPIx == SPI2) {
		clock_periph = LL_APB1_GRP1_PERIPH_SPI2;
		spi_irq_type = SPI2_IRQn;
	}
	else if(SPIx == SPI3) {
		clock_periph = LL_APB1_GRP1_PERIPH_SPI3;
		spi_irq_type = SPI3_IRQn;
	}
	else { return; }

	LL_APB1_GRP1_EnableClock(clock_periph);

	NVIC_SetPriority(spi_irq_type, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 4, 4));
	NVIC_EnableIRQ(spi_irq_type);

	LL_SPI_InitTypeDef SPI_InitStruct = {0};
	SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
	SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
	SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
	SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
	SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
	SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
	SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV16;	//1MHz
	// SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV32;	//500kHz
	SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
	SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
	SPI_InitStruct.CRCPoly = 10;
	LL_SPI_Init(SPIx, &SPI_InitStruct);
	LL_SPI_SetStandard(SPIx, LL_SPI_PROTOCOL_MOTOROLA);

	// LL_SPI_EnableIT_RXNE(SPIx);
	// LL_SPI_EnableIT_TXE(SPIx);
	// LL_SPI_EnableIT_ERR(SPIx);
	
	//! Interrupt
	// NVIC_SetPriority(SPIx, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 4, 8));
	// NVIC_EnableIRQ(SPIx);	

	LL_SPI_Enable(SPIx);

}



//* SPI *******************************************************************
extern uint8_t spi_send_byte_waiting(SPI_TypeDef *SPIx, uint8_t data) {
	uint8_t reg = 0;
	LL_SPI_TransmitData8(SPIx, data);
	// printf("SPI Send: %u\r\n", data);
	while(LL_SPI_IsActiveFlag_RXNE(SPIx) == 0 || LL_SPI_IsActiveFlag_BSY(SPIx));
	reg = LL_SPI_ReceiveData8(SPIx);
	// printf("SPI: 0x%02X\r\n", reg);
	return reg;
}


extern void spi_send_multiple_bytes_waiting(SPI_TypeDef *SPIx, uint8_t* write_data, uint8_t* read_data, uint8_t size) {
	for(uint8_t i = 0; i < size; i++) {
		if(write_data != NULL) 	{ LL_SPI_TransmitData8(SPIx, write_data[i]); }
		else 					{ LL_SPI_TransmitData8(SPIx, SPI_NOP); }
		while(LL_SPI_IsActiveFlag_RXNE(SPIx) == 0 || LL_SPI_IsActiveFlag_BSY(SPIx));

		if(read_data != NULL) 	{ read_data[i] = LL_SPI_ReceiveData8(SPIx); }
		else 					{ LL_SPI_ReceiveData8(SPIx); }
	}
}


// void SPI2_IRQHandler(void) {
// 	uint8_t data;
// 	/* Check RXNE flag value in ISR register */
// 	if(LL_SPI_IsActiveFlag_RXNE(SPIx)) {
// 		/* Call function Slave Reception Callback */
// 		// SPI1_Rx_Callback();
// 		data = LL_SPI_ReceiveData8(SPIx);
// 		printf("SPI RX: %u\r\n", data);
// 	}
// 	/* Check TXNE flag value in ISR register */
// 	else if(LL_SPI_IsActiveFlag_TXE(SPIx)) {
// 		/* Call function Slave Reception Callback */
// 		// SPI1_Tx_Callback();
// 		printf("SPI TX Done\r\n");
// 	}
// 	/* Check STOP flag value in ISR register */
// 	else if(LL_SPI_IsActiveFlag_OVR(SPIx)) {
// 		/* Call Error function */
// 		// SPI1_TransferError_Callback();
// 		printf("SPI Error\r\n");
// 	}
// }
