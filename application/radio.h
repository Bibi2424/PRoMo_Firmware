#ifndef RADIO_H
#define RADIO_H


typedef void (*get_data_ptr)(uint8_t*, uint8_t);
typedef void (*on_connection_lost_ptr)(void);


typedef struct {
	uint8_t radio_rx_id;
	uint8_t radio_tx_id;
	get_data_ptr get_data;
	on_connection_lost_ptr on_connection_lost;
} radio_settings_t;


extern bool radio_init(radio_settings_t *settings);
// extern bool radio_is_rx_ready(void);
// extern bool radio_process_rx(void);
extern void radio_loop(void);
extern void radio_set_ack_payload(uint8_t *data, uint8_t size);

#endif
