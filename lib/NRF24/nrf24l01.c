#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "nrf24l01.h"
#include "time.h"
#include "spi.h"
#include "gpio.h"



#define CONFIG_FOR_POWER_DOWN 	(NRF24L01_CONFIG_CRC_ENABLE | NRF24L01_CONFIG_CRC_2BIT | NRF24L01_CONFIG_POWER_DOWN | NRF24L01_CONFIG_PRIM_TX)
#define CONFIG_FOR_POWER_UP 	(NRF24L01_CONFIG_CRC_ENABLE | NRF24L01_CONFIG_CRC_2BIT | NRF24L01_CONFIG_POWER_UP | NRF24L01_CONFIG_PRIM_TX)
#define CONFIG_FOR_TX_MODE 		(CONFIG_FOR_POWER_UP)
#define CONFIG_FOR_RX_MODE 		(NRF24L01_CONFIG_CRC_ENABLE | NRF24L01_CONFIG_CRC_2BIT | NRF24L01_CONFIG_POWER_UP | NRF24L01_CONFIG_PRIM_RX)

#define DEFAULT_TX_TIMEOUT		100
#define NRF_DEFAULT_TIMEOUT		10

static nrf24l01_config_t nrf24l01_config = {
	.rx_address = "PROM0",
	.channel = 76,
	.payload_size = 32,
};

volatile static bool nrf_isr = false;
static bool nrf_lock_acquired = false;
static nrf24l01_status_t last_nrf_status = {0};
static uint8_t nrf_config_register = CONFIG_FOR_POWER_DOWN;


static void nrf_check_on_isr(void);
static bool wait_for_tx_end(uint32_t timeout_ms);
static void prepare_radio_for_tx(bool ack);

static bool nrf_lock_acquire_blocking(uint32_t timeout_ms) {
	uint32_t start_time = millis();
	while(nrf_lock_acquired == true){
		if(millis() - start_time > timeout_ms) {
			return false;
		}
	}
	nrf_lock_acquired = true;
	return true;
}

static void nrf_lock_release(void) {
	nrf_lock_acquired = false;
}


static void nrf24l01_gpio_init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct;
	LL_EXTI_InitTypeDef EXTI_InitStruct;

	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	//! CE - PB10, CSN - PB12
	LL_GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.Pin = NRF_CE_Pin | NRF_CSN_Pin;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(NRF_CE_GPIO_Port, &GPIO_InitStruct);
	CE_LOW;
	CSN_HIGH;

	//! IRQ - PB2
	LL_GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.Pin = NRF_IRQ_Pin;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(NRF_IRQ_GPIO_Port, &GPIO_InitStruct);

	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTB, LL_SYSCFG_EXTI_LINE2);
	EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_2;
	EXTI_InitStruct.LineCommand = ENABLE;
	EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
	EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
	LL_EXTI_Init(&EXTI_InitStruct);

	NVIC_SetPriority(EXTI2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 4, 0));
	NVIC_EnableIRQ(EXTI2_IRQn);

	//! SCK - PB13, MISO - PB14, MOSI - PB15
	LL_GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.Pin = NRF_SCK_Pin | NRF_MOSI_Pin | NRF_MISO_Pin;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_5;
	LL_GPIO_Init(NRF_SCK_GPIO_Port, &GPIO_InitStruct);
}


//* IRQ Line *******************************************************************
void EXTI2_IRQHandler(void) {
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_2) != RESET) {
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_2);
		// debugf("NRF INT\n");
		nrf_isr = true;
	}
}

//* NRF Low Level *******************************************************************
extern uint8_t nrf_read_register(uint8_t reg) {
	reg &= 0x1f;
	CSN_LOW;
	spi2_send_byte_waiting(NRF24L01_COMMAND_READ_REGISTER | reg, NULL);
	uint8_t value;
	spi2_send_byte_waiting(0xff, &value);
	CSN_HIGH;
	return value;
}


extern void nrf_write_register(uint8_t reg, uint8_t value) {
	reg &= 0x1f;
	CSN_LOW;
	spi2_send_byte_waiting(NRF24L01_COMMAND_WRITE_REGISTER | reg, NULL);
	spi2_send_byte_waiting(value, NULL);
	CSN_HIGH;
}


extern void nrf_read_multiple_bytes_register(uint8_t reg, uint8_t *data, uint8_t size) {
	reg &= 0x1f;
	CSN_LOW;
	spi2_send_byte_waiting(NRF24L01_COMMAND_READ_REGISTER | reg, NULL);
	spi2_send_multiple_bytes_waiting(NULL, data, size);
	CSN_HIGH;
}


extern void nrf_write_multiple_bytes_register(uint8_t reg, uint8_t *data, uint8_t size) {
	reg &= 0x1f;
	CSN_LOW;
	spi2_send_byte_waiting(NRF24L01_COMMAND_WRITE_REGISTER | reg, NULL);
	spi2_send_multiple_bytes_waiting(data, NULL, size);
	CSN_HIGH;
}

//* NRF Mid Level *******************************************************************
static uint8_t _nrf_get_status(void) {
	CSN_LOW;
	uint8_t status;
	spi2_send_byte_waiting(NRF24L01_COMMAND_NOP, &status);
	CSN_HIGH;
	return status;
}
extern uint8_t nrf_get_status(void) { 
	if(nrf_lock_acquire_blocking(NRF_DEFAULT_TIMEOUT) == false) { return 0; }
	uint8_t ret = _nrf_get_status(); 
	nrf_lock_release(); 
	return ret; 
}


static void _nrf_flush_rx_buffer(void) {
	CSN_LOW;
	spi2_send_byte_waiting(NRF24L01_COMMAND_FLUSH_RX, NULL);
	CSN_HIGH;
}
extern void nrf_flush_rx_buffer(void) { 
	if(nrf_lock_acquire_blocking(NRF_DEFAULT_TIMEOUT) == false) { return; };
	_nrf_flush_rx_buffer(); 
	nrf_lock_release(); 
}


static void _nrf_flush_tx_buffer(void) {
	CSN_LOW;
	spi2_send_byte_waiting(NRF24L01_COMMAND_FLUSH_TX, NULL);
	CSN_HIGH;	
}
extern void nrf_flush_tx_buffer(void) { 
	if(nrf_lock_acquire_blocking(NRF_DEFAULT_TIMEOUT) == false) { return; };
	_nrf_flush_tx_buffer(); 
	nrf_lock_release(); 
}


static void _nrf_clear_interrupt(void) {
	nrf_write_register(NRF24L01_REGISTER_STATUS,
		NRF24L01_STATUS_DATA_READY_INT | NRF24L01_STATUS_DATA_SENT_INT | NRF24L01_STATUS_MAX_RT_INT
	);
}
extern void nrf_clear_interrupt(void) { 
	if(nrf_lock_acquire_blocking(NRF_DEFAULT_TIMEOUT) == false) { return; };
	_nrf_clear_interrupt(); 
	nrf_lock_release(); 
}


static void _nrf_power_down(void) {
	CE_LOW;
	nrf_write_register(NRF24L01_REGISTER_CONFIG, CONFIG_FOR_POWER_DOWN);
	nrf_config_register = CONFIG_FOR_POWER_DOWN;
}
extern void nrf_power_down(void) { 
	if(nrf_lock_acquire_blocking(NRF_DEFAULT_TIMEOUT) == false) { return; };
	_nrf_power_down(); 
	nrf_lock_release(); 
}


static void _nrf_power_up(void) {
	if (!(nrf_config_register & NRF24L01_CONFIG_POWER_UP)) {
		nrf_config_register |= NRF24L01_CONFIG_POWER_UP;
		nrf_write_register(NRF24L01_REGISTER_CONFIG, nrf_config_register);

		LL_mDelay(NRF_POWERUP_DELAY);
	}
}
extern void nrf_power_up(void) { 
	if(nrf_lock_acquire_blocking(NRF_DEFAULT_TIMEOUT) == false) { return; };
	_nrf_power_up(); 
	nrf_lock_release(); 
}


static void _nrf_start_rx(void) {
	_nrf_power_up();

    BIT_SET(nrf_config_register, NRF24L01_CONFIG_PRIM_BIT);
    // debugf("C:[%02X]\n", nrf_config_register);
    nrf_write_register(NRF24L01_REGISTER_CONFIG, nrf_config_register);
    _nrf_clear_interrupt();
    CE_HIGH;

	LL_mDelay(5);

    // close reading pipe 0
    nrf_write_register(NRF24L01_REGISTER_EN_RXADDR, nrf_read_register(NRF24L01_REGISTER_EN_RXADDR) & ~NRF24L01_ENABLE_PIPE(0));
}
extern void nrf_start_rx(void) { 
	if(nrf_lock_acquire_blocking(NRF_DEFAULT_TIMEOUT) == false) { return; };
	_nrf_start_rx(); 
	nrf_lock_release(); 
}


static void _nrf_stop_rx(void) {
	CE_LOW;
	LL_mDelay(1);

	BIT_CLEAR(nrf_config_register, NRF24L01_CONFIG_PRIM_BIT);
	nrf_write_register(NRF24L01_REGISTER_CONFIG, nrf_config_register);

	// Enable RX on pipe0
	nrf_write_register(NRF24L01_REGISTER_EN_RXADDR, nrf_read_register(NRF24L01_REGISTER_EN_RXADDR) | NRF24L01_ENABLE_PIPE(0));
}
extern void nrf_stop_rx(void) { 
	if(nrf_lock_acquire_blocking(NRF_DEFAULT_TIMEOUT) == false) { return; };
	_nrf_stop_rx(); 
	nrf_lock_release(); 
}

//* NRF High Level *******************************************************************
extern bool nrf_init(uint8_t radio_rx_id, uint8_t radio_tx_id) {
	nrf24l01_gpio_init();
	spi2_init();
	LL_mDelay(10);

	if(nrf_lock_acquire_blocking(NRF_DEFAULT_TIMEOUT) == false) { return false; }

	// Set RF settings
	nrf_write_register(NRF24L01_REGISTER_RF_SETUP, NRF24L01_RF_SETTINGS_1MBPS | NRF24L01_RF_SETTINGS_0DBM);
	// Set retransmission
	nrf_write_register(NRF24L01_REGISTER_SETUP_RETR, NRF24L01_RETR_DELAY_IN_250MS(2) | NRF24L01_RETR_COUNT(15));
	// Set rx_address
	nrf24l01_config.rx_address[4] = radio_rx_id;
	nrf_write_multiple_bytes_register(NRF24L01_REGISTER_RX_ADDR_P1, nrf24l01_config.rx_address, 5);
	// Set tx_address
	uint8_t tx_address[5];
	memcpy(tx_address, nrf24l01_config.rx_address, 5);
	tx_address[4] = radio_tx_id;
	nrf_write_multiple_bytes_register(NRF24L01_REGISTER_RX_ADDR_P0, tx_address, 5);
	nrf_write_multiple_bytes_register(NRF24L01_REGISTER_TX_ADDR, tx_address, 5);
	// Set Dynamic payload length
	nrf_write_register(NRF24L01_REGISTER_DYNPD, NRF24L01_ENABLE_PIPE(0) | NRF24L01_ENABLE_PIPE(1));
	// Enable dynamic payload size, ACK
	nrf_write_register(NRF24L01_REGISTER_FEATURE,
		NRF24L01_FEATURE_NOACK_PAYLOAD_ENABLE | NRF24L01_FEATURE_ACK_ENABLE | NRF24L01_FEATURE_DYN_PAYLAOD_ENABLE
	);
	// Enable auto ack
	nrf_write_register(NRF24L01_REGISTER_EN_AA, NRF24L01_ENABLE_PIPE(0) | NRF24L01_ENABLE_PIPE(1));
	// Enable rx address
	nrf_write_register(NRF24L01_REGISTER_EN_RXADDR, NRF24L01_ENABLE_PIPE(0) | NRF24L01_ENABLE_PIPE(1));
	// Set maximum size for all rx pipes
	for(uint8_t i = 0; i < 6; i++) {
		nrf_write_register(NRF24L01_REGISTER_RX_PW_P0 + i, 32);
	}
	// address width 5bits (per default)
	nrf_write_register(NRF24L01_REGISTER_SETUP_AW, 0b11);
	// Set chanel
	nrf_write_register(NRF24L01_REGISTER_RF_CH, nrf24l01_config.channel);

	// Flush buffers
	_nrf_flush_rx_buffer();
	_nrf_flush_tx_buffer();
	// Clear Pending Interrupt
	_nrf_clear_interrupt();
	// Prepare config register for standby mode
	nrf_config_register = CONFIG_FOR_POWER_DOWN;
	nrf_write_register(NRF24L01_REGISTER_CONFIG, nrf_config_register);
	nrf_config_register = nrf_read_register(NRF24L01_REGISTER_CONFIG);

	_nrf_power_up();

	nrf_lock_release();
	if(nrf_config_register != CONFIG_FOR_POWER_UP) {
		return false;
	}
	return true;
}


extern void nrf_loop(void) {
	if(nrf_isr == true) {
		if(nrf_lock_acquire_blocking(NRF_DEFAULT_TIMEOUT) == false) { return; }
		nrf_check_on_isr();
		nrf_lock_release();
		nrf_isr = false;
	}
}


static inline void nrf_check_on_isr(void) {
	last_nrf_status.value = nrf_read_register(NRF24L01_REGISTER_STATUS);
	// debugf("[%02X]", last_nrf_status.value);
	last_nrf_status.value &= (NRF24L01_STATUS_DATA_READY_INT | NRF24L01_STATUS_DATA_SENT_INT | NRF24L01_STATUS_MAX_RT_INT);
	//! Clear any set interrupt bit
	if(last_nrf_status.value) { nrf_write_register(NRF24L01_REGISTER_STATUS, last_nrf_status.value); }
}


extern nrf24l01_status_t nrf_has_data_isr(void) {
	return last_nrf_status;
}


extern void nrf_set_my_address(uint8_t *address, uint8_t size) {
	if(nrf_lock_acquire_blocking(NRF_DEFAULT_TIMEOUT) == false) { return; }
	memcpy(&nrf24l01_config.rx_address[5-size], address, size);
	nrf_write_multiple_bytes_register(NRF24L01_REGISTER_RX_ADDR_P1, nrf24l01_config.rx_address, 5);
	nrf_lock_release();
}


extern uint8_t nrf_has_data(void) {
	if(nrf_lock_acquire_blocking(NRF_DEFAULT_TIMEOUT) == false) { return 0; }

	if(nrf_config_register != CONFIG_FOR_RX_MODE) {
		// debugf("SET RX");
		_nrf_start_rx();
	}

	uint8_t status = nrf_read_register(NRF24L01_REGISTER_STATUS);

	if(status & NRF24L01_STATUS_RX_PIPE_NO) { //! Data in pipe 1
		CSN_LOW;
		spi2_send_byte_waiting(NRF24L01_COMMAND_R_RX_PL_WID, NULL);
		uint8_t rx_size;
		spi2_send_byte_waiting(0xff, &rx_size);
		CSN_HIGH;
		if(rx_size <= 32) { 
			nrf_lock_release();
			return rx_size; 
		}
	}
	nrf_lock_release();
	return 0;
}


extern uint8_t nrf_read_data(uint8_t *data) {
	if(nrf_lock_acquire_blocking(NRF_DEFAULT_TIMEOUT) == false) { return 0; }
	//! Get data length
	CSN_LOW;
	spi2_send_byte_waiting(NRF24L01_COMMAND_R_RX_PL_WID, NULL);
	uint8_t rx_size;
	spi2_send_byte_waiting(0xff, &rx_size);
	CSN_HIGH;

	//! Read data
	CSN_LOW;
	spi2_send_byte_waiting(NRF24L01_COMMAND_READ_RX_PAYLOAD, NULL);
	spi2_send_multiple_bytes_waiting(NULL, data, rx_size);
	CSN_HIGH;

	//! Clear relevant interrupt flag
	uint8_t status_reg = nrf_read_register(NRF24L01_REGISTER_STATUS);
	if(status_reg & NRF24L01_STATUS_DATA_READY_INT) {
		nrf_write_register(NRF24L01_REGISTER_STATUS, (status_reg | NRF24L01_STATUS_DATA_READY_INT));
	}
	last_nrf_status.rx_ready = 0;
	
	nrf_lock_release();
	return rx_size;
}


extern bool nrf_write_data(uint8_t *data, uint8_t data_size, bool ack) {
	if(nrf_lock_acquire_blocking(NRF_DEFAULT_TIMEOUT) == false) { return false; }
	//! Put radio in tx
	if((nrf_config_register & BIT_VALUE(NRF24L01_CONFIG_PRIM_BIT)) != 0) {
		// printf("NOT IN TX\n");
		prepare_radio_for_tx(ack);
	}

	//! Clear tx related bits in STATUS register
	nrf_write_register(NRF24L01_REGISTER_STATUS, NRF24L01_STATUS_DATA_SENT_INT | NRF24L01_STATUS_MAX_RT_INT);

	CSN_LOW;
	if(ack == false) { spi2_send_byte_waiting(NRF24L01_COMMAND_W_TX_PAYLOAD_NO_ACK, NULL); }
	else { spi2_send_byte_waiting(NRF24L01_COMMAND_WRITE_TX_PAYLOAD, NULL); }
	spi2_send_multiple_bytes_waiting(data, NULL, data_size);
	CSN_HIGH;

	CE_HIGH;

	bool ret = wait_for_tx_end(DEFAULT_TX_TIMEOUT);
	nrf_lock_release();
	return ret;
}


static void prepare_radio_for_tx(bool ack) {
	//! Verify radio in TX mode
	if((nrf_config_register & BIT_VALUE(NRF24L01_CONFIG_PRIM_BIT)) != 0) {
		// debugf("Switch to TX\n");
		CE_LOW;
		BIT_CLEAR(nrf_config_register, NRF24L01_CONFIG_PRIM_BIT);
		nrf_write_register(NRF24L01_REGISTER_CONFIG, nrf_config_register);
		LL_mDelay(5);
	}

	uint8_t status = nrf_read_register(NRF24L01_REGISTER_FIFO_STATUS);
	if(ack && (status & NRF24L01_FIFO_RX_FULL)) {
		nrf_flush_rx_buffer();
	}
	if(status & NRF24L01_FIFO_TX_FULL) {
		wait_for_tx_end(DEFAULT_TX_TIMEOUT);
	}
}


static bool wait_for_tx_end(uint32_t timeout_ms) {
	uint32_t start_time = millis();

	while(last_nrf_status.tx_ok == 0 && last_nrf_status.tx_fail == 0) {
		if(millis() - start_time > timeout_ms) {
			// debugf("[timeout]");
			break;
		}
	}

	CE_LOW;
	_nrf_clear_interrupt();

	if(last_nrf_status.tx_ok) {
		last_nrf_status.tx_ok = 0;
		// debugf("[tx ok]");
		return true;
	}
	else if(last_nrf_status.tx_fail) {
		last_nrf_status.tx_fail = 0;
		_nrf_flush_tx_buffer();
		// debugf("[max rt]");
		return false;
	}

	return false;
}


extern bool nrf_write_ack_data(uint8_t *data, uint8_t data_size) {
	if(nrf_lock_acquire_blocking(NRF_DEFAULT_TIMEOUT) == false) { return false; }
	const uint8_t pipe = 1;

	CSN_LOW;
	spi2_send_byte_waiting(NRF24L01_COMMAND_W_ACK_PAYLOAD | pipe, NULL);
	spi2_send_multiple_bytes_waiting(data, NULL, data_size);
	CSN_HIGH;

	// TODO: Can we switch to last_nrf_status.tx_full ??
	uint8_t nrf_status = _nrf_get_status();

	nrf_lock_release();
    return !(nrf_status & BIT_VALUE(NRF24L01_STATUS_TX_FULL_INT));
}
