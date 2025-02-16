#define DEBUG_THIS_FILE DEBUG_RADIO_FILE

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "utils.h"
#include "debug.h"

#include "radio.h"
#include "nrf24l01.h"
#include "scheduler.h"
#include "gpio.h"


static radio_settings_t *current_settings;
volatile static bool remote_connection_alive = false;


static void on_connection_lost(void) {
    remote_connection_alive = false;
    current_settings->on_connection_lost();
}


extern bool radio_init(radio_settings_t *settings) {
    current_settings = settings;

    bool res;
    res = nrf_init(current_settings->radio_rx_id, current_settings->radio_tx_id);
    if(res == false) { return false; }

    nrf_start_rx();
    return true;
}


static bool radio_is_rx_ready(void) {
    nrf24l01_status_t nrf_status = nrf_has_data_isr();
    return nrf_status.rx_ready == 1 ? true: false;
}


static bool radio_process_rx(void) {
    nrf24l01_status_t nrf_status = nrf_has_data_isr();

    if(nrf_status.rx_ready == 0) {
        return false;
    }

    // debugf("#");
    uint8_t nrf_rx_size = 0;
    uint8_t nrf_data[32] = {0};
    remote_connection_alive = true;
    nrf_rx_size = nrf_read_data(nrf_data);
    nrf_flush_rx_buffer();

    current_settings->get_data(nrf_data, nrf_rx_size);

    SET_PIN(LD2_GPIO_Port, LD2_Pin, 1);
    scheduler_add_event(SCHEDULER_TASK_LED2, 50*SCHEDULER_MS, SCHEDULER_ONE_SHOT, blink_led2);

    scheduler_remove_event(SCHEDULER_TASK_LOST_CONNECTION);
    scheduler_add_event(SCHEDULER_TASK_LOST_CONNECTION, 500*SCHEDULER_MS, SCHEDULER_ONE_SHOT, on_connection_lost);
    return true;
}


extern void radio_loop(void) {
    nrf_loop();
    if(radio_is_rx_ready()) {
        radio_process_rx();
    }
}


extern void radio_set_ack_payload(uint8_t *data, uint8_t size) {
    if(data == NULL || size == 0) { return; }
    if(remote_connection_alive == false) { return; }

    bool res = nrf_write_ack_data(data, size);
    if(!res) {
        nrf_flush_tx_buffer();
        debugf("TXFULL\n");
    }
}
