#define DEBUG_THIS_FILE     DEBUG_MAIN_FILE

#include <stdint.h>
#include <stdbool.h>

#include "global.h"
#include "debug.h"

#include "main.h"
#include "usart.h"
#include "gpio.h"
#include "encoder.h"
#include "motor.h"
#include "radio.h"
#include "nrf24l01.h"
#include "scheduler.h"
#include "VL53L0X.h"
#include "sensors.h"
#include "control_loop.h"
#include "pid_controller.h"
#include "ws2812b.h"
#include "mpu_6050.h"


static void sensors_get_event(void);
static void get_data_from_radio(uint8_t *data, uint8_t size);
static void lost_connection(void);
static void SystemClock_Config(void);



volatile static bool gpio_pressed = false;



extern int main(void) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    SystemClock_Config();

    //! Note: Need to set the SysTick interrupt priority after SystemClock_Config() or it doesn't work
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));           /* SysTick_IRQn interrupt configuration */
    NVIC_EnableIRQ(SysTick_IRQn);

    MX_GPIO_Init();
    #if(DEBUG_UART == 1)
        MX_USART1_UART_Init(DEBUG_BAUDRATE);
    #elif(DEBUG_UART == 6)
        MX_USART6_UART_Init(DEBUG_BAUDRATE);
    #endif
    setbuf(stdout, NULL);       //! For unbuffered ouput
    debugf("\n**************************************\n");
    debugf(  "* " xstr(TARGET) "fw v" xstr(FW_VERSION_MAJOR) "." xstr(FW_VERSION_MINOR) "." xstr(FW_VERSION_REV) "\n");
    debugf(  "* " xstr(HW_TYPE) "\n" );
    debugf(  "* System_Frequency: %lu MHz\n", SystemCoreClock);
    debugf(  "**************************************\n");
    debugf("Booting...\n");

    scheduler_init();
    scheduler_add_event(SCHEDULER_TASK_LED1, 1*SECOND, SCHEDULER_ALWAYS, blink_led1);

    ws2812b_init();

    if(mpu_6050_init(MPU_6050_DEFAULT_ADDRESS)) { debugf("\t- MPU6050 Init OK\n"); }
    else { debugf("\t- MPU6050 Init Error\n"); }

    encoders_init();
    motors_init();

    if(sensors_vl53l0x_init()) { debugf("\t- VL53 Init OK\n"); }
    else { debugf("\t- VL53 Init error\n"); }
    // scheduler_add_event(SCHEDULER_TASK_VL53_GET, 250*MS, SCHEDULER_ALWAYS, sensors_get_event);

    static radio_settings_t radio_settings = {
        .radio_rx_id = 2,
        .get_data = get_data_from_radio,
        .on_connection_lost = lost_connection,
    };
    if(radio_init(&radio_settings)) { debugf("\t- NRF Init OK\n"); }
    else { debugf("\t- NRF Init Error\n"); }

    debugf("Init Done\n");


    uint32_t radio_last_execution = 0;
    uint32_t motor_control_last_execution = 0;
    uint32_t aleds_last_execution = 0;
    uint32_t mpu_last_execution = 0;
    rgb_t strip[5] = {{25, 0, 0}, {0, 25, 0}, {0, 0, 25}, {0, 0, 0}};

    while (1) {
        uint32_t current_time = millis();

        if(gpio_pressed) {
            gpio_pressed = false;
            debugf("Press\n");

            // uint8_t tx_data[5] = { 0xAA, 0x55, 0xff };
            // bool res = nrf_write_data(1, tx_data, 3, true);
            // debugf("Sending... %u\n", res);
        }

        if(radio_is_rx_ready()) {
            radio_process_rx();
        }

        if(MOTOR_CONTROL_INTERVAL_MS > 0 && current_time - motor_control_last_execution > MOTOR_CONTROL_INTERVAL_MS) {
            motor_control_last_execution = current_time;

            do_control_loop();
        }

        if(MPU_INTERVAL_MS > 0 && current_time - mpu_last_execution > MPU_INTERVAL_MS) {
            mpu_last_execution = current_time;

            mpu_data_t mpu_data;
            mpu_6050_read_all(&mpu_data);
            debugf("Accel: %d/%d/%d\n", mpu_data.accel.x, mpu_data.accel.y, mpu_data.accel.z);
            debugf("Gyro: %d/%d/%d\n", mpu_data.gyro.x, mpu_data.gyro.y, mpu_data.gyro.z);
        }

        if(ALEDS_INTERVAL_MS > 0 && current_time - aleds_last_execution > ALEDS_INTERVAL_MS) {
            aleds_last_execution = current_time;

            for(int8_t i = 4; i >= 0; i--) {
                strip[i+1] = strip[i];
            }
            strip[0] = strip[4];
            ws2812b_send(strip, 4);
        }

    }
    return 0;
}


extern void blink_led1(void) {
    TOGGLE_PIN(LD1_GPIO_Port, LD1_Pin);
}


extern void blink_led2(void) {
    TOGGLE_PIN(LD2_GPIO_Port, LD2_Pin);
}


static void sensors_get_event(void) {

    // statInfo_t range;
    // uint16_t result = sensors_vl53l0x_get_next(&range);
    // debugf("VL53 - %u\n", result);

    // statInfo_t range;
    // uint16_t result = sensors_vl53l0x_range_one(VL53L0X_FRONT_ADDRESS, &range);
    // debugf("VL53 - %u\n", result);

    statInfo_t ranges[4];
    sensors_vl53l0x_get_all(ranges);
    debugf("VL53 - L: %u, F: %u, R: %u, B: %u\n", ranges[1].rawDistance, ranges[0].rawDistance, ranges[2].rawDistance, ranges[3].rawDistance);
}


static void get_data_from_radio(uint8_t *data, uint8_t size) {

    int8_t forward_speed = (int8_t)data[0];
    int8_t steer_speed = (int8_t)data[1];
    // debugf("NRF: %d - %d\n", forward_speed, steer_speed);

    int8_t speed_left = forward_speed + (int8_t)steer_speed / 3;
    int8_t speed_right = forward_speed - (int8_t)steer_speed / 3;
    // debugf("NRF: %d - %d\n", speed_left, speed_right);
    set_target_speed(speed_left, speed_right);
}


static void lost_connection(void) {
    set_target_speed(0, 0);
    debugf("Lost Connection with Remote\n");
}


static void SystemClock_Config(void) {
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);
    while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_3) { }
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    LL_RCC_HSE_Enable();

    /* Wait till HSE is ready */
    while(LL_RCC_HSE_IsReady() != 1) { }
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_15, 144, LL_RCC_PLLP_DIV_2);
    LL_RCC_PLL_ConfigDomain_48M(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_15, 144, LL_RCC_PLLQ_DIV_5);
    LL_RCC_PLL_Enable();

    /* Wait till PLL is ready */
    while(LL_RCC_PLL_IsReady() != 1) { }
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    /* Wait till System clock is ready */
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) { }
    LL_Init1msTick(120000000);
    LL_SetSystemCoreClock(120000000);
    SysTick_Config(SystemCoreClock / 1000);

    /* Update the time base */
    // if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK) { Error_Handler(); }
    // LL_RCC_SetTIMPrescaler(LL_RCC_TIM_PRESCALER_TWICE);
}


extern void UserButton_Callback(void) {
    gpio_pressed = true;
}


extern void _Error_Handler(char *file, int line) {
    printf("Error\n");
    while(1) {}
}


volatile static uint32_t counter = 0;
extern uint32_t millis(void) {
    return counter;
}


void SysTick_Handler(void) {
    // TOGGLE_PIN(DEBUG_Pin_Port, DEBUG_Pin_1);
    counter++;
}


#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) { 
    printf("Wrong parameters value: file %s on line %lu\n", file, line);
}
#endif
