#define DEBUG_THIS_FILE   DEBUG_SPI_FILE

#include "global.h"
#include "debug.h"

#include "spi.h"
#include "gpio.h"

#define SPI_NOP		0xFF


extern void spi_init(SPI_TypeDef *SPIx) {
	uint32_t clock_periph;
	// IRQn_Type spi_irq_type;

	if(SPIx == SPI1) {
		clock_periph = LL_APB2_GRP1_PERIPH_SPI1;
		// spi_irq_type = SPI1_IRQn;
	}
	else if(SPIx == SPI2) {
		clock_periph = LL_APB1_GRP1_PERIPH_SPI2;
		// spi_irq_type = SPI2_IRQn;
	}
	else if(SPIx == SPI3) {
		clock_periph = LL_APB1_GRP1_PERIPH_SPI3;
		// spi_irq_type = SPI3_IRQn;
	}
	else { return; }

	LL_APB1_GRP1_EnableClock(clock_periph);

	LL_SPI_InitTypeDef SPI_InitStruct = {0};
	SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
	SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
	SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
	SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
	SPI_InitStruct.ClockPhase = LL_SPI_PHASE_1EDGE;
	SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
	//! NOTE: SPI1 - APB2, SPI2/SPI3 - APB1
	// SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV8;	//! SPI2: 3.75MHz for APB1=30MHz
	SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV16;	//! SPI2: 1.875MHz for APB1=30MHz
	SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
	SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
	SPI_InitStruct.CRCPoly = 10;
	LL_SPI_Init(SPIx, &SPI_InitStruct);
	LL_SPI_SetStandard(SPIx, LL_SPI_PROTOCOL_MOTOROLA);

	// LL_SPI_EnableIT_RXNE(SPIx);
	// LL_SPI_EnableIT_TXE(SPIx);
	// LL_SPI_EnableIT_ERR(SPIx);
	
	//! Interrupt
	// NVIC_SetPriority(spi_irq_type, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 4, 4));
	// NVIC_EnableIRQ(spi_irq_type);	

	LL_SPI_Enable(SPIx);

}



//* SPI *******************************************************************
#define SPI_DEFAULT_TIMEOUT		5

static bool spi_wait_received(SPI_TypeDef *SPIx, uint32_t timeout) {
	while(LL_SPI_IsActiveFlag_RXNE(SPIx) == 0 || LL_SPI_IsActiveFlag_BSY(SPIx)) {
        if (!LL_SYSTICK_IsActiveCounterFlag()) { continue; }
        if(--timeout == 0) {
        	printf("SPI timeout\n");
            return false;
        }
    }
    return true;
}


extern bool spi_send_byte_waiting(SPI_TypeDef *SPIx, uint8_t write_data, uint8_t *read_data) {
	LL_SPI_TransmitData8(SPIx, write_data);
	if(spi_wait_received(SPIx, SPI_DEFAULT_TIMEOUT) == false) { return false; }

	if(read_data != NULL) 	{ *read_data = LL_SPI_ReceiveData8(SPIx); }
	else 					{ LL_SPI_ReceiveData8(SPIx); }
	return true;
}


extern bool spi_send_multiple_bytes_waiting(SPI_TypeDef *SPIx, uint8_t* write_data, uint8_t* read_data, uint8_t size) {
	for(uint8_t i = 0; i < size; i++) {
		if(write_data != NULL) 	{ LL_SPI_TransmitData8(SPIx, write_data[i]); }
		else 					{ LL_SPI_TransmitData8(SPIx, SPI_NOP); }
		if(spi_wait_received(SPIx, SPI_DEFAULT_TIMEOUT) == false) { return false; }

		if(read_data != NULL) 	{ read_data[i] = LL_SPI_ReceiveData8(SPIx); }
		else 					{ LL_SPI_ReceiveData8(SPIx); }
	}
	return true;
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
