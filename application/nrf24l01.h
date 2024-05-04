#ifndef NRF24L01_H
#define NRF24L01_H

#include "main.h"

#ifndef NRF_POWERUP_DELAY
#define NRF_POWERUP_DELAY	5
#endif

#ifndef BIT_VALUE
#define BIT_VALUE(n)	(1 << (n))
#endif
#ifndef BIT_SET
#define BIT_SET(reg, n)	(reg |= BIT_VALUE(n))
#endif
#ifndef BIT_CLEAR
#define BIT_CLEAR(reg, n)	(reg &= ~BIT_VALUE(n))
#endif

//! NRF commands
#define NRF24L01_COMMAND_READ_REGISTER			0x00
#define NRF24L01_COMMAND_WRITE_REGISTER			0x20
#define NRF24L01_COMMAND_READ_RX_PAYLOAD		0x61
#define NRF24L01_COMMAND_WRITE_TX_PAYLOAD		0xA0
#define NRF24L01_COMMAND_FLUSH_TX				0xE1
#define NRF24L01_COMMAND_FLUSH_RX				0xE2
#define NRF24L01_COMMAND_REUSE_TX_PL			0xE3
#define NRF24L01_COMMAND_ACTIVATE				0x50
#define NRF24L01_COMMAND_R_RX_PL_WID			0x60
#define NRF24L01_COMMAND_W_ACK_PAYLOAD			0xA8
#define NRF24L01_COMMAND_W_TX_PAYLOAD_NO_ACK	0xB0
#define NRF24L01_COMMAND_NOP					0xFF

//! NRF Registers
#define NRF24L01_REGISTER_CONFIG		0x00
#define NRF24L01_REGISTER_EN_AA			0x01
#define NRF24L01_REGISTER_EN_RXADDR		0x02
#define NRF24L01_REGISTER_SETUP_AW		0x03
#define NRF24L01_REGISTER_SETUP_RETR	0x04
#define NRF24L01_REGISTER_RF_CH			0x05
#define NRF24L01_REGISTER_RF_SETUP		0x06
#define NRF24L01_REGISTER_STATUS		0x07
#define NRF24L01_REGISTER_OBSERVE_TX	0x08
#define NRF24L01_REGISTER_CD			0x09
#define NRF24L01_REGISTER_RX_ADDR_P0	0x0A
#define NRF24L01_REGISTER_RX_ADDR_P1	0x0B
#define NRF24L01_REGISTER_RX_ADDR_P2	0x0C
#define NRF24L01_REGISTER_RX_ADDR_P3	0x0D
#define NRF24L01_REGISTER_RX_ADDR_P4	0x0E
#define NRF24L01_REGISTER_RX_ADDR_P5	0x0F
#define NRF24L01_REGISTER_TX_ADDR		0x10
#define NRF24L01_REGISTER_RX_PW_P0		0x11
#define NRF24L01_REGISTER_RX_PW_P1		0x12
#define NRF24L01_REGISTER_RX_PW_P2		0x13
#define NRF24L01_REGISTER_RX_PW_P3		0x14
#define NRF24L01_REGISTER_RX_PW_P4		0x15
#define NRF24L01_REGISTER_RX_PW_P5		0x16
#define NRF24L01_REGISTER_FIFO_STATUS	0x17
#define NRF24L01_REGISTER_DYNPD			0x1C
#define NRF24L01_REGISTER_FEATURE		0x1D

// Params for the CONFIG Register
#define NRF24L01_CONFIG_RX_DR_INT_BIT	6
#define NRF24L01_CONFIG_RX_DR_INT_ON	0x00	// Default
#define NRF24L01_CONFIG_RX_DR_INT_OFF	0x40

#define NRF24L01_CONFIG_TX_DS_INT_BIT	5
#define NRF24L01_CONFIG_TX_DS_INT_ON	0x00	// Default
#define NRF24L01_CONFIG_TX_DS_INT_OFF	0x20

#define NRF24L01_CONFIG_MAX_RT_INT_BIT	4
#define NRF24L01_CONFIG_MAX_RT_INT_ON	0x00	// Default
#define NRF24L01_CONFIG_MAX_RT_INT_OFF	0x10

#define NRF24L01_CONFIG_CRC_EN_BIT		3
#define NRF24L01_CONFIG_CRC_DISABLE		0x00
#define NRF24L01_CONFIG_CRC_ENABLE		0x08	// Default

#define NRF24L01_CONFIG_CRC_SIZE_BIT	2
#define NRF24L01_CONFIG_CRC_1BIT		0x00	// Default
#define NRF24L01_CONFIG_CRC_2BIT		0x04

#define NRF24L01_CONFIG_POWER_BIT		1
#define NRF24L01_CONFIG_POWER_DOWN		0x00	// Default
#define NRF24L01_CONFIG_POWER_UP		0x02

#define NRF24L01_CONFIG_PRIM_BIT		0
#define NRF24L01_CONFIG_PRIM_TX			0x00	// Default
#define NRF24L01_CONFIG_PRIM_RX			0x01

//Params for STATUS register
#define NRF24L01_STATUS_DATA_READY_INT	0x40
#define NRF24L01_STATUS_DATA_SENT_INT	0x20
#define NRF24L01_STATUS_MAX_RT_INT		0x10
#define NRF24L01_STATUS_RX_PIPE_NO		0x01 	// 3bits
#define NRF24L01_STATUS_TX_FULL_INT		0x00

// Params for the AUTO Retransmission Register
#define NRF24L01_RETR_DELAY_IN_250MS(x) ((x & 0x0F) << 4)
#define NRF24L01_RETR_COUNT(x) 			(x & 0x0F)

// Params for the RF_SETTINGS Register
#define NRF24L01_RF_SETTINGS_1MBPS		0x00
#define NRF24L01_RF_SETTINGS_2MBPS		0x08	// Default

#define NRF24L01_RF_SETTINGS_18DBM		0x00
#define NRF24L01_RF_SETTINGS_12DBM		0x02
#define NRF24L01_RF_SETTINGS_6DBM		0x04
#define NRF24L01_RF_SETTINGS_0DBM		0x06	// Default

// Params for Pipe Registers
#define NRF24L01_ENABLE_PIPE(x)	(1 << (x & 0x3F))

// Params for the Feature Register
#define NRF24L01_FEATURE_NOACK_PAYLOAD_DISABLE	0x00
#define NRF24L01_FEATURE_NOACK_PAYLOAD_ENABLE	0x01

#define NRF24L01_FEATURE_ACK_DISABLE			0x00
#define NRF24L01_FEATURE_ACK_ENABLE				0x02

#define NRF24L01_FEATURE_DYN_PAYLAOD_DISABLE	0x00
#define NRF24L01_FEATURE_DYN_PAYLAOD_ENABLE		0x04

// Params for the FIFO Register
#define NRF24L01_FIFO_TX_REUSE			(1 << 6)
#define NRF24L01_FIFO_TX_FULL			(1 << 5)
#define NRF24L01_FIFO_TX_EMPTY			(1 << 4)
#define NRF24L01_FIFO_RX_FULL			(1 << 1)
#define NRF24L01_FIFO_RX_EMPTY			(1 << 0)


#define CE_LOW		LL_GPIO_ResetOutputPin(NRF_CE_GPIO_Port, NRF_CE_Pin)
#define CE_HIGH		LL_GPIO_SetOutputPin(NRF_CE_GPIO_Port, NRF_CE_Pin);
#define CSN_LOW		LL_GPIO_ResetOutputPin(NRF_CSN_GPIO_Port, NRF_CSN_Pin);
#define CSN_HIGH	LL_GPIO_SetOutputPin(NRF_CSN_GPIO_Port, NRF_CSN_Pin);


typedef struct nrf24l01_config {
	uint8_t rx_address[5];
	uint8_t channel;
	uint8_t payload_size;
} nrf24l01_config_t;

typedef union nrf24l01_status {
	struct {
		uint8_t tx_full : 1;
		uint8_t unused : 3;
		uint8_t tx_fail : 1;
		uint8_t tx_ok : 1;
		uint8_t rx_ready : 1;	
	};
	uint8_t value;
} nrf24l01_status_t;

// #if sizeof(nrf24l01_status_t) != 1
// 	#error "Check nrf24l01_status_t bit field (should be 1B)."
// #endif



//* NRF Low Level *******************************************************************
extern uint8_t nrf_read_register(uint8_t reg);
extern void nrf_write_register(uint8_t reg, uint8_t value);
extern void nrf_read_multiple_bytes_register(uint8_t reg, uint8_t *data, uint8_t size);
extern void nrf_write_multiple_bytes_register(uint8_t reg, uint8_t *data, uint8_t size);
//* NRF High Level *******************************************************************
extern uint8_t nrf_get_status(void);
extern void nrf_flush_rx_buffer(void);
extern void nrf_flush_tx_buffer(void);
extern void nrf_clear_interrupt(void);
extern void nrf_power_down(void);
extern void nrf_power_up(void);
extern void nrf_stop_rx(void);
extern void nrf_start_rx(void);
//* NRF High Level *******************************************************************
extern bool nrf_init(uint8_t radio_rx_id, uint8_t radio_tx_id);
extern void nrf_loop(void);
extern nrf24l01_status_t nrf_has_data_isr(void);
extern uint8_t nrf_has_data(void);
extern uint8_t nrf_read_data(uint8_t *data);
extern bool nrf_write_data(uint8_t *data, uint8_t data_size, bool ack);
extern bool nrf_write_ack_data(uint8_t *data, uint8_t data_size);

#endif