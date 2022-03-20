#define DEBUG_THIS_FILE DEBUG_RADIO_FILE

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "global.h"
#include "debug.h"

#include "radio.h"
#include "nrf24l01.h"
#include "scheduler.h"
#include "gpio.h"


uint8_t nrf_rx_size = 0;
uint8_t nrf_data[32] = {0};
static radio_settings_t *current_settings;


extern bool radio_init(radio_settings_t *settings) {
    current_settings = settings;

    bool res;
    res = nrf_init(current_settings->radio_rx_id);
    if(res == false) { return false; }

    nrf_start_rx();
    return true;
}


extern bool radio_is_rx_ready(void) {
    return nrf_has_data_isr()->rx_ready ? true: false;
}


extern void radio_process_rx(void) {
    nrf24l01_status_t * nrf_status = nrf_has_data_isr();

    if(nrf_status->rx_ready) {
        debugf("#");
        nrf_status->rx_ready = 0;
        nrf_rx_size = nrf_read_data(nrf_data);
        nrf_flush_rx_buffer();

        current_settings->get_data(nrf_data, nrf_rx_size);

        SET_PIN(LD2_GPIO_Port, LD2_Pin, 1);
        scheduler_add_event(SCHEDULER_TASK_LED2, 50*MS, SCHEDULER_ONE_SHOT, blink_led2);

        scheduler_remove_event(SCHEDULER_TASK_LOST_CONNECTION);
        scheduler_add_event(SCHEDULER_TASK_LOST_CONNECTION, 200*MS, SCHEDULER_ONE_SHOT, current_settings->on_connection_lost);
    }
}
