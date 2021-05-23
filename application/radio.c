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


static void lost_connection(void);


extern void radio_init(void) {
    nrf_init(2);
    nrf_set_rx_mode();
}


extern void radio_run(void) {
    //! Remote 
    // debugf("NRF status: [%02X]\n", nrf_get_status());

    nrf24l01_status_t * nrf_status = nrf_has_data_isr(); 
    if(nrf_status->rx_ready) {
        nrf_status->rx_ready = 0;
        nrf_rx_size = nrf_read_data(nrf_data);

        int8_t forward_speed = (int8_t)nrf_data[0];
        int8_t steer_speed = (int8_t)nrf_data[1];
        debugf("NRF: %d - %d\n", forward_speed, steer_speed);

        // target_speed_left = forward_speed + (int32_t)steer_speed / 3;
        // target_speed_right = forward_speed - (int32_t)steer_speed / 3;
        // debugf("NRF: %ld - %ld\n", target_speed_left, target_speed_right);

        SET_PIN(LD2_GPIO_Port, LD2_Pin, 1);
        scheduler_add_event(SCHEDULER_TASK_LED2, 50*MS, SCHEDULER_ONE_SHOT, blink_led2);

        scheduler_remove_event(SCHEDULER_TASK_LOST_CONNECTION);
        scheduler_add_event(SCHEDULER_TASK_LOST_CONNECTION, 200*MS, SCHEDULER_ONE_SHOT, lost_connection);
    }
}


static void lost_connection(void) {
    // target_speed_left = 0;
    // target_speed_right = 0;
    // motor_left_set_speed(0);
    // motor_right_set_speed(0);
    debugf("Lost Connection with Remote\r\n");
}