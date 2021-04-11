#define DEBUG_THIS_FILE	DEBUG_NRF24L01_FILE

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "global.h"
#include "debug.h"

#include "nrf24l01.h"
#include "gpio.h"



#define CONFIG_FOR_RX_MODE 	(NRF24L01_CONFIG_CRC_ENABLE | NRF24L01_CONFIG_POWER_UP | NRF24L01_CONFIG_PRIM_RX)



static nrf24l01_config_t nrf24l01_config = {
	.rx_address = {1, 2, 3, 4, 0},
	.channel = 100,
	.payload_size = 32,
};

volatile static nrf24l01_status_t last_nrf_status = {0};


static void nrf_check_on_isr(void);
static bool wait_for_tx_end(void);
static void prepare_radio_for_tx(uint8_t radio_tx_id, bool ack);



//! TODO: Split the file into SPI and NRF stuff
extern bool SPI2_NRF24L01_Init(uint8_t radio_rx_id) {
	LL_GPIO_InitTypeDef GPIO_InitStruct;
	LL_EXTI_InitTypeDef EXTI_InitStruct;

	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	//! CE - PB10, CSN - PB12
	GPIO_InitStruct.Pin = NRF_CE_Pin | NRF_CSN_Pin;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(NRF_CE_GPIO_Port, &GPIO_InitStruct);
	CE_LOW;
	CSN_HIGH;

	//! IRQ - PB2
	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE2);
	EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_2;
	EXTI_InitStruct.LineCommand = ENABLE;
	EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
	EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
	LL_EXTI_Init(&EXTI_InitStruct);

  	LL_GPIO_SetPinPull(NRF_IRQ_GPIO_Port, NRF_IRQ_Pin, LL_GPIO_PULL_NO);
  	LL_GPIO_SetPinMode(NRF_IRQ_GPIO_Port, NRF_IRQ_Pin, LL_GPIO_MODE_INPUT);

	NVIC_SetPriority(EXTI2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 4, 8));
	NVIC_EnableIRQ(EXTI2_IRQn);

	//! SCK - PB13, MISO - PB14, MOSI - PB15
	GPIO_InitStruct.Pin = NRF_SCK_Pin | NRF_MOSI_Pin | NRF_MISO_Pin;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
	LL_GPIO_Init(NRF_SCK_GPIO_Port, &GPIO_InitStruct);

	//! Interrupt
	// NVIC_SetPriority(SPI2_IRQn, 0);
	// NVIC_EnableIRQ(SPI2_IRQn);

	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);

	NVIC_SetPriority(SPI2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 4, 4));
	NVIC_EnableIRQ(SPI2_IRQn);

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
	LL_SPI_Init(SPI2, &SPI_InitStruct);
	LL_SPI_SetStandard(SPI2, LL_SPI_PROTOCOL_MOTOROLA);

	// LL_SPI_EnableIT_RXNE(SPI2);
	// LL_SPI_EnableIT_TXE(SPI2);
	// LL_SPI_EnableIT_ERR(SPI2);

	LL_SPI_Enable(SPI2);
	LL_mDelay(100);

	//! NOTE: Need reset of all register
	// Set chanel
	nrf_write_register(NRF24L01_REGISTER_RF_CH, nrf24l01_config.channel);
	// Set RF settings
	nrf_write_register(NRF24L01_REGISTER_RF_SETUP, NRF24L01_RF_SETTINGS_2MBPS | NRF24L01_RF_SETTINGS_0DBM);
	// Set retransmission
	// nrf_write_register(NRF24L01_REGISTER_SETUP_RETR, NRF24L01_RETR_DELAY_IN_250MS(2) | NRF24L01_RETR_COUNT(15));
	nrf_write_register(NRF24L01_REGISTER_SETUP_RETR, NRF24L01_RETR_DELAY_IN_250MS(1) | NRF24L01_RETR_COUNT(15));
	// Set rx_address;
	nrf24l01_config.rx_address[4] = radio_rx_id;
	nrf_write_multiple_bytes_register(NRF24L01_REGISTER_RX_ADDR_P1, nrf24l01_config.rx_address, 5);
	// Set Dynamic payload length
	nrf_write_register(NRF24L01_REGISTER_DYNPD, NRF24L01_DYNPD_ENABLE_PIPE(0) | NRF24L01_DYNPD_ENABLE_PIPE(1));
	// Enable dynamic payload size, ACK
	nrf_write_register(NRF24L01_REGISTER_FEATURE, 
		NRF24L01_FEATURE_NOACK_PAYLOAD_ENABLE | NRF24L01_FEATURE_ACK_ENABLE | NRF24L01_FEATURE_DYN_PAYLAOD_ENABLE
		// 0x01 | 0x02 | 0x041
	);
	// Set pipeline size
	// nrf_write_register(NRF24L01_REGISTER_RX_PW_P0, nrf24l01_config.payload_size);
	// nrf_write_register(NRF24L01_REGISTER_RX_PW_P1, nrf24l01_config.payload_size);
	// nrf_write_register(NRF24L01_REGISTER_RX_PW_P2, nrf24l01_config.payload_size);
	// nrf_write_register(NRF24L01_REGISTER_RX_PW_P3, nrf24l01_config.payload_size);
	// nrf_write_register(NRF24L01_REGISTER_RX_PW_P4, nrf24l01_config.payload_size);
	// nrf_write_register(NRF24L01_REGISTER_RX_PW_P5, nrf24l01_config.payload_size);
	// Flush buffers
	nrf_flush_rx_buffer();
	nrf_flush_tx_buffer();
	// Clear Pending Interrupt
	nrf_clear_interrupt();
	// Set Config
	// uint8_t config = 	NRF24L01_CONFIG_RX_DR_INT_ON | NRF24L01_CONFIG_TX_DS_INT_ON | NRF24L01_CONFIG_MAX_RT_INT_ON |
	// 					NRF24L01_CONFIG_CRC_ENABLE | NRF24L01_CONFIG_CRC_1BIT | NRF24L01_CONFIG_POWER_UP | NRF24L01_CONFIG_PRIM_RX;
	// nrf_write_register(NRF24L01_REGISTER_CONFIG,
	// 	config
	// 	// NRF24L01_CONFIG_RX_DR_INT_ON | NRF24L01_CONFIG_TX_DS_INT_ON | NRF24L01_CONFIG_MAX_RT_INT_ON |
	// 	// NRF24L01_CONFIG_CRC_ENABLE | NRF24L01_CONFIG_CRC_1BIT | NRF24L01_CONFIG_POWER_UP | NRF24L01_CONFIG_PRIM_RX
	// );

	if(nrf_set_rx_mode() == FALSE) {
		debugf("NRF Init Fail\r\n");
		return FALSE;
	}

	debugf("NRF Init OK\r\n");
	return TRUE;
}


//* IRQ Line *******************************************************************
void EXTI2_IRQHandler(void) {
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_2) != RESET) {
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_2);
		// debugf("NRF INT\n");
		nrf_check_on_isr();
	}
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


// void SPI2_IRQHandler(void) {
// 	uint8_t data;
// 	/* Check RXNE flag value in ISR register */
// 	if(LL_SPI_IsActiveFlag_RXNE(SPI2)) {
// 		/* Call function Slave Reception Callback */
// 		// SPI1_Rx_Callback();
// 		data = LL_SPI_ReceiveData8(SPI2);
// 		printf("SPI RX: %u\r\n", data);
// 	}
// 	/* Check TXNE flag value in ISR register */
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


extern void nrf_set_my_address(uint8_t *address, uint8_t size) {
	memcpy(&nrf24l01_config.rx_address[5-size], address, size);
	nrf_write_multiple_bytes_register(NRF24L01_REGISTER_RX_ADDR_P1, nrf24l01_config.rx_address, 5);
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



extern uint8_t nrf_set_rx_mode(void) {
	wait_for_tx_end();
	debugf("Switch to rX - ");

	CE_LOW;
	nrf_write_register(NRF24L01_REGISTER_CONFIG, CONFIG_FOR_RX_MODE);
	// nrf_write_register(NRF24L01_REGISTER_CONFIG, NRF24L01_CONFIG_POWER_UP);
	// nrf_write_register(NRF24L01_REGISTER_CONFIG, NRF24L01_CONFIG_PRIM_RX);
	CE_HIGH;

	LL_mDelay(5);

	if(nrf_read_register(NRF24L01_REGISTER_CONFIG) == CONFIG_FOR_RX_MODE) {
		debugf("OK\n");
		return TRUE;
	}
	else {
		debugf("FAIL\n");
		return FALSE;
	}
}


extern nrf24l01_status_t * nrf_has_data_isr(void) {
	return &last_nrf_status;
}


extern uint8_t nrf_has_data(void) {
	uint8_t config = nrf_read_register(NRF24L01_REGISTER_CONFIG);
	if((config & 0x0f) != (NRF24L01_CONFIG_CRC_ENABLE | NRF24L01_CONFIG_CRC_1BIT | NRF24L01_CONFIG_POWER_UP | NRF24L01_CONFIG_PRIM_RX)) {
		nrf_set_rx_mode();
	}

	if(((nrf_read_register(NRF24L01_REGISTER_STATUS) >> 1) & 0x07) == 1) { //! Data in pipe 1

		CSN_LOW;
		spi_send_byte_waiting(NRF24L01_COMMAND_R_RX_PL_WID);
		uint8_t rx_size = spi_send_byte_waiting(0xff);
		CSN_HIGH;
		if(rx_size <= 32) { return rx_size; }
	}
	return 0;
}


extern uint8_t nrf_read_data(uint8_t *data) {

	CSN_LOW;
	spi_send_byte_waiting(NRF24L01_COMMAND_R_RX_PL_WID);
	uint8_t rx_size = spi_send_byte_waiting(0xff);
	CSN_HIGH;

	LL_mDelay(1);

	CSN_LOW;
	spi_send_byte_waiting(NRF24L01_COMMAND_READ_RX_PAYLOAD);
	spi_send_multiple_bytes_waiting(NULL, data, rx_size);
	CSN_HIGH;

    uint8_t status_reg = nrf_read_register(NRF24L01_REGISTER_STATUS);
    if(status_reg & NRF24L01_STATUS_DATA_READY_INT) {
    	nrf_write_register(NRF24L01_REGISTER_STATUS, (status_reg | NRF24L01_STATUS_DATA_READY_INT));
    }

    return rx_size;

	// uint8_t status;
	// CSN_LOW;
	// status = spi_send_byte_waiting(NRF24L01_COMMAND_R_RX_PL_WID);
	// // debugf("NRF status: %02X\n", status);
	// uint8_t rx_size = spi_send_byte_waiting(0xff);
	// CSN_HIGH;
	
	// if(rx_size == 0 || !(status & NRF24L01_STATUS_DATA_READY_INT)) { return 0; }

	// CE_LOW;
	// LL_mDelay(1);

	// CSN_LOW;
	// status = spi_send_byte_waiting(NRF24L01_COMMAND_READ_RX_PAYLOAD);
	// spi_send_multiple_bytes_waiting(NULL, data, rx_size);
	// CSN_HIGH;


	// nrf_clear_interrupt();
	// // nrf_flush_rx_buffer();
	// //! Clear tx buffer from ack payloadnrf_get_status
	// nrf_flush_tx_buffer();

	// //! Clear NRF24L01_STATUS_DATA_READY_INT
	// nrf_write_register(NRF24L01_REGISTER_STATUS, status | NRF24L01_STATUS_DATA_READY_INT);

	// LL_mDelay(1);
	// CE_HIGH;
	// return rx_size;
}


extern bool nrf_write_data(uint8_t radio_tx_id, uint8_t *data, uint8_t data_size, bool ack) {
	//! Put radio in tx
	prepare_radio_for_tx(radio_tx_id, ack);

	//! Clear tx related bits in STATUS register
	nrf_write_register(NRF24L01_REGISTER_STATUS, NRF24L01_STATUS_DATA_SENT_INT | NRF24L01_STATUS_MAX_RT_INT);

	CSN_LOW;
	if(ack == false) { spi_send_byte_waiting(NRF24L01_COMMAND_W_TX_PAYLOAD_NO_ACK); }
	else { spi_send_byte_waiting(NRF24L01_COMMAND_WRITE_TX_PAYLOAD); }
	spi_send_multiple_bytes_waiting(data, NULL, data_size);
	CSN_HIGH;

	return wait_for_tx_end();

}


static void nrf_check_on_isr(void) {
	last_nrf_status.value = nrf_read_register(NRF24L01_REGISTER_STATUS);
	last_nrf_status.value &= (NRF24L01_STATUS_DATA_READY_INT | NRF24L01_STATUS_DATA_SENT_INT | NRF24L01_STATUS_MAX_RT_INT);
	//! Clear any set interrupt bit
	if(last_nrf_status.value) { nrf_write_register(NRF24L01_REGISTER_STATUS, last_nrf_status.value); }
}


static void prepare_radio_for_tx(uint8_t radio_tx_id, bool ack) {
	static uint8_t last_radio_tx_id = 0;

	//! CHeck radio TX ID
	if(last_radio_tx_id != radio_tx_id) {
		last_radio_tx_id = radio_tx_id;

		uint8_t * p_addr = nrf24l01_config.rx_address;
		uint8_t tx_address[5] = { p_addr[0], p_addr[1], p_addr[2], p_addr[3], radio_tx_id };
		// memcpy(tx_address, nrf24l01_config.rx_address, 4);
		// tx_address[4]
		nrf_write_multiple_bytes_register(NRF24L01_REGISTER_RX_ADDR_P0, tx_address, 5);
		nrf_write_multiple_bytes_register(NRF24L01_REGISTER_TX_ADDR, tx_address, 5);
	}

	//! Verify radio in TX mode
	uint8_t config = nrf_read_register(NRF24L01_REGISTER_CONFIG);
	if((config & NRF24L01_CONFIG_PRIM_RX) == 1) {
		debugf("Switch to TX\n");
		CE_LOW;
		nrf_write_register(NRF24L01_REGISTER_CONFIG, NRF24L01_CONFIG_CRC_ENABLE | NRF24L01_CONFIG_CRC_1BIT | NRF24L01_CONFIG_POWER_UP | NRF24L01_CONFIG_PRIM_TX);
		LL_mDelay(5);
	}

	uint8_t status = nrf_read_register(NRF24L01_REGISTER_FIFO_STATUS);
	if(ack && (status & NRF24L01_FIFO_RX_FULL)) {
		nrf_flush_rx_buffer();
	}
	if(status & NRF24L01_FIFO_TX_FULL) {
		wait_for_tx_end();
	}
}


static bool wait_for_tx_end(void) {
	// _resetInterruptFlags = 0; // Disable interrupt flag reset logic in 'whatHappened'.

	uint8_t result = false;
	uint8_t reg;
	uint8_t tx_remaining_attempt = 90;

	while(tx_remaining_attempt-- > 0) {
		reg = nrf_read_register(NRF24L01_REGISTER_FIFO_STATUS);
		if(reg & NRF24L01_FIFO_TX_EMPTY) {
			result = true;
			break;
		}

		CE_HIGH;
		LL_mDelay(10);
		CE_LOW;

		LL_mDelay(1); //! 600uS

		reg = nrf_read_register(NRF24L01_REGISTER_STATUS);
		if(reg & NRF24L01_STATUS_DATA_SENT_INT) {
			nrf_write_register(NRF24L01_REGISTER_STATUS, NRF24L01_STATUS_DATA_SENT_INT);

		}
		else if(reg & NRF24L01_STATUS_MAX_RT_INT) {
			spi_send_byte_waiting(NRF24L01_COMMAND_FLUSH_TX);
			nrf_write_register(NRF24L01_REGISTER_STATUS, NRF24L01_STATUS_MAX_RT_INT);
			break;
		}
	}

    // _resetInterruptFlags = 1; // Re-enable interrupt reset logic in 'whatHappened'.
    return result;
}

