#define DEBUG_THIS_FILE	DEBUG_NRF24L01_FILE

#include "nrf24l01.h"
#include "global.h"
#include "gpio.h"
#include "main.h"

#include "stm32f4xx_ll_spi.h"

static nrf24l01_config nrf24l01_default_config = {
	.rx_address = {1, 2, 3, 4, 0},
	.channel = 100,
	.payload_size = 32,
};

//! TODO: Split the file into SPI and NRF stuff
extern void SPI2_NRF24L01_Init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct;

	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	//! CE - PB2, CSN - PC12
	GPIO_InitStruct.Pin = NRF_CE_Pin | NRF_CSN_Pin;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	CE_LOW;
	CSN_HIGH;

	//! SCK - PB13, MISO - PB14, MOSI - PB15
	GPIO_InitStruct.Pin = NRF_SCK_Pin | NRF_MOSI_Pin | NRF_MISO_Pin;
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
	LL_mDelay(100);

	//! NOTE: Need reset of all register
	// Set rx_address
	nrf_write_multiple_bytes_register(NRF24L01_REGISTER_RX_ADDR_P1, nrf24l01_default_config.rx_address, 5);
	// Set chanel
	nrf_write_register(NRF24L01_REGISTER_RF_CH, nrf24l01_default_config.channel);
	// Set pipeline size
	// nrf_write_register(NRF24L01_REGISTER_RX_PW_P0, &nrf24l01_default_config.payload_size);
	// nrf_write_register(NRF24L01_REGISTER_RX_PW_P1, &nrf24l01_default_config.payload_size);
	// nrf_write_register(NRF24L01_REGISTER_RX_PW_P2, &nrf24l01_default_config.payload_size);
	// nrf_write_register(NRF24L01_REGISTER_RX_PW_P3, &nrf24l01_default_config.payload_size);
	// nrf_write_register(NRF24L01_REGISTER_RX_PW_P4, &nrf24l01_default_config.payload_size);
	// nrf_write_register(NRF24L01_REGISTER_RX_PW_P5, &nrf24l01_default_config.payload_size);
	// Set RF settings
	nrf_write_register(NRF24L01_REGISTER_RF_SETUP, NRF24L01_RF_SETTINGS_2MBPS | NRF24L01_RF_SETTINGS_0DBM);
	// Set retransmission
	nrf_write_register(NRF24L01_REGISTER_SETUP_RETR, NRF24L01_RETR_DELAY_IN_250MS(2) | NRF24L01_RETR_COUNT(15));
	// Set Dynamic payload length
	nrf_write_register(NRF24L01_REGISTER_DYNPD, NRF24L01_DYNPD_ENABLE_PIPE(0) | NRF24L01_DYNPD_ENABLE_PIPE(1));
	// Enable dynamic payload size, ACK
	nrf_write_register(NRF24L01_REGISTER_FEATURE, 
		NRF24L01_FEATURE_NOACK_PAYLOAD_ENABLE | NRF24L01_FEATURE_ACK_ENABLE | NRF24L01_FEATURE_DYN_PAYLAOD_ENABLE
	);
	// Flush buffers
	nrf_flush_rx_buffer();
	nrf_flush_tx_buffer();
	// Clear Pending Interrupt
	nrf_clear_interrupt();
	// Set Config
	uint8_t config = 	NRF24L01_CONFIG_RX_DR_INT_ON | NRF24L01_CONFIG_TX_DS_INT_ON | NRF24L01_CONFIG_MAX_RT_INT_ON |
						NRF24L01_CONFIG_CRC_ENABLE | NRF24L01_CONFIG_CRC_1BIT | NRF24L01_CONFIG_POWER_UP | NRF24L01_CONFIG_PRIM_RX;
	nrf_write_register(NRF24L01_REGISTER_CONFIG,
		config
		// NRF24L01_CONFIG_RX_DR_INT_ON | NRF24L01_CONFIG_TX_DS_INT_ON | NRF24L01_CONFIG_MAX_RT_INT_ON |
		// NRF24L01_CONFIG_CRC_ENABLE | NRF24L01_CONFIG_CRC_1BIT | NRF24L01_CONFIG_POWER_UP | NRF24L01_CONFIG_PRIM_RX
	);

	CE_HIGH;
	LL_mDelay(5);

	if(config != nrf_read_register(NRF24L01_REGISTER_CONFIG)) {
		printf("NRF Init Fail\r\n");
		return;
	}
	printf("NRF Init OK\r\n");
	// CE_HIGH;
}


//* SPI *******************************************************************
extern uint8_t spi_send_byte_waiting(uint8_t data) {
	uint8_t reg = 0;
	LL_SPI_TransmitData8(SPI2, data);
	// printf("SPI Send: %u\r\n", data);
	while(LL_SPI_IsActiveFlag_RXNE(SPI2) == 0 || LL_SPI_IsActiveFlag_BSY(SPI2));
	reg = LL_SPI_ReceiveData8(SPI2);
	// printf("SPI: 0x%02X\r\n", reg);
	return reg;
}


extern void spi_send_multiple_bytes_waiting(uint8_t* write_data, uint8_t* read_data, uint8_t size) {
	for(uint8_t i = 0; i < size; i++) {
		if(write_data != NULL) 	{ LL_SPI_TransmitData8(SPI2, write_data[i]); }
		else 					{ LL_SPI_TransmitData8(SPI2, NRF24L01_COMMAND_NOP); }
		while(LL_SPI_IsActiveFlag_RXNE(SPI2) == 0 || LL_SPI_IsActiveFlag_BSY(SPI2));

		if(read_data != NULL) 	{ read_data[i] = LL_SPI_ReceiveData8(SPI2); }
		else 					{ LL_SPI_ReceiveData8(SPI2); }
	}
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


//* NRF Low Level *******************************************************************
extern uint8_t nrf_read_register(uint8_t reg) {
	reg &= 0x1f;
	CSN_LOW;
	spi_send_byte_waiting(NRF24L01_COMMAND_READ_REGISTER | reg);
	uint8_t value = spi_send_byte_waiting(0xff);
	CSN_HIGH;
	return value;
}


extern void nrf_write_register(uint8_t reg, uint8_t value) {
	reg &= 0x1f;
	CSN_LOW;
	spi_send_byte_waiting(NRF24L01_COMMAND_WRITE_REGISTER | reg);
	spi_send_byte_waiting(value);
	CSN_HIGH;
}


extern void nrf_read_multiple_bytes_register(uint8_t reg, uint8_t *data, uint8_t size) {
	reg &= 0x1f;
	CSN_LOW;
	spi_send_byte_waiting(NRF24L01_COMMAND_READ_REGISTER | reg);
	spi_send_multiple_bytes_waiting(NULL, data, size);
	CSN_HIGH;
}


extern void nrf_write_multiple_bytes_register(uint8_t reg, uint8_t *data, uint8_t size) {
	reg &= 0x1f;
	CSN_LOW;
	spi_send_byte_waiting(NRF24L01_COMMAND_WRITE_REGISTER | reg);
	spi_send_multiple_bytes_waiting(data, NULL, size);
	CSN_HIGH;
}

//* NRF High Level *******************************************************************
extern uint8_t nrf_get_status(void) {
	CSN_LOW;
	return spi_send_byte_waiting(NRF24L01_COMMAND_NOP);
	CSN_HIGH;
}


extern void nrf_set_my_address(uint8_t *address) {
	nrf_write_multiple_bytes_register(NRF24L01_REGISTER_RX_ADDR_P1, address, 5);
}


extern void nrf_flush_rx_buffer(void) {
	CSN_LOW;
	spi_send_byte_waiting(NRF24L01_COMMAND_FLUSH_RX);
	CSN_HIGH;
}


extern void nrf_flush_tx_buffer(void) {
	CSN_LOW;
	spi_send_byte_waiting(NRF24L01_COMMAND_FLUSH_TX);
	CSN_HIGH;
}


extern void nrf_clear_interrupt(void) {
	nrf_write_register(NRF24L01_REGISTER_STATUS, 
		NRF24L01_STATUS_DATA_READY_INT | NRF24L01_STATUS_DATA_SENT_INT | NRF24L01_STATUS_MAX_RT_INT
	);
}


extern uint8_t nrf_read_data(uint8_t *data) {
	uint8_t status;
	CSN_LOW;
	status = spi_send_byte_waiting(NRF24L01_COMMAND_R_RX_PL_WID);
	uint8_t rx_size = spi_send_byte_waiting(0xff);
	CSN_HIGH;
	
	if(rx_size == 0 || !(status & NRF24L01_STATUS_DATA_READY_INT)) { return 0; }

	CE_LOW;
	LL_mDelay(1);

	CSN_LOW;
	status = spi_send_byte_waiting(NRF24L01_COMMAND_READ_RX_PAYLOAD);
	spi_send_multiple_bytes_waiting(NULL, data, rx_size);
	CSN_HIGH;


	nrf_clear_interrupt();
	// nrf_flush_rx_buffer();
	//! Clear tx buffer from ack payloadnrf_get_status
	nrf_flush_tx_buffer();

	//! Clear NRF24L01_STATUS_DATA_READY_INT
	nrf_write_register(NRF24L01_REGISTER_STATUS, status | NRF24L01_STATUS_DATA_READY_INT);

	LL_mDelay(1);
	CE_HIGH;
	return rx_size;
}

