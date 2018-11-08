#ifndef NRF24L01_H
#define NRF24L01_H

#include "main.h"


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


#define CE_LOW		LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_2)
#define CE_HIGH		LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_2);
#define CSN_LOW		LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_12);
#define CSN_HIGH	LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_12);


extern void SPI2_NRF24L01_Init(void);

//* NRF Low Level *******************************************************************
extern uint8_t nrf_read_register(uint8_t reg);
extern void nrf_write_register(uint8_t reg, uint8_t value);
extern void nrf_read_multiple_bytes_register(uint8_t reg, uint8_t *data, uint8_t size);
extern void nrf_write_multiple_bytes_register(uint8_t reg, uint8_t *data, uint8_t size);
//* NRF High Level *******************************************************************
extern uint8_t nrf_get_status(void);

#endif