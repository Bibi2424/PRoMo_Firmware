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
static radio_settings_t current_settings;


// static void lost_connection(void);


extern void radio_init(radio_settings_t *settings) {
    current_settings = *settings;

    nrf_init(current_settings.radio_rx_id);
    nrf_set_rx_mode();
}


extern void radio_run(void) {
    //! Remote 
    // debugf("NRF status: [%02X]\n", nrf_get_status());

    nrf24l01_status_t * nrf_status = nrf_has_data_isr(); 
    if(nrf_status->rx_ready) {
        nrf_status->rx_ready = 0;
        nrf_rx_size = nrf_read_data(nrf_data);

        current_settings.get_data(nrf_data, nrf_rx_size);


        SET_PIN(LD2_GPIO_Port, LD2_Pin, 1);
        scheduler_add_event(SCHEDULER_TASK_LED2, 50*MS, SCHEDULER_ONE_SHOT, blink_led2);

        scheduler_remove_event(SCHEDULER_TASK_LOST_CONNECTION);
        scheduler_add_event(SCHEDULER_TASK_LOST_CONNECTION, 200*MS, SCHEDULER_ONE_SHOT, current_settings.on_connection_lost);
    }
}
