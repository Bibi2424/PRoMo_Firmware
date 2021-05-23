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
#include "i2c.h"
#include "nrf24l01.h"
#include "scheduler.h"
#include "VL53L0X.h"
#include "sensors.h"
#include "pid_controller.h"


static void LL_Init(void);
void SystemClock_Config(void);



#define NRF_INTERVAL                10
static uint32_t nrf_last_execution = 0;
#define MOTOR_CONTROL_INTERVAL      50
static uint32_t motor_control_last_execution = 0;

static int32_t target_speed_left = 0, target_speed_right = 0;

volatile static bool gpio_pressed = false;


extern void blink_led1(void) {
    TOGGLE_PIN(LD1_GPIO_Port, LD1_Pin);
}


extern void blink_led2(void) {
    TOGGLE_PIN(LD2_GPIO_Port, LD2_Pin);
}


static void lost_connection(void) {
    target_speed_left = 0;
    target_speed_right = 0;
    // motor_left_set_speed(0);
    // motor_right_set_speed(0);
    printf("Lost Connection with Remote\r\n");
}


static uint8_t tx_data[5] = { 0xAA, 0x55, 0xff };

extern int main(void) {
    LL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    #if(DEBUG_UART == 1)
        MX_USART1_UART_Init(DEBUG_BAUDRATE);
    #elif(DEBUG_UART == 6)
        MX_USART6_UART_Init(DEBUG_BAUDRATE);
    #endif
    setbuf(stdout, NULL);       //! For unbuffered ouput
    debugf("\r\n**************************************\r\n");
    debugf(    "* " xstr(TARGET) " v" xstr(FW_VERSION_MAJOR) "." xstr(FW_VERSION_MINOR) "." xstr(FW_VERSION_REV) " - " xstr(HW_TYPE)  "\r\n");
    debugf(    "* System_Frequency: %lu MHz\n", SystemCoreClock);
    debugf(    "* Booting...\r\n");
    debugf(    "**************************************\r\n");

    scheduler_init();
    scheduler_add_event(SCHEDULER_TASK_LED1, 1*SECOND, SCHEDULER_ALWAYS, blink_led1);

    TIM34_Encoder_Init();
    TIM2_Motor_Init();

    SPI2_NRF24L01_Init(2);

    sensors_VL53L0X_init();

    debugf("Init Done\r\n");

    uint8_t nrf_rx_size = 0;
    uint8_t nrf_data[32] = {0};
    nrf_set_rx_mode();

    statInfo_t xTraStats;
    statInfo_t ranges[4];

    pid_controller_t pid_speed_left = {
        .compute_interval = 50,
        .proportional_gain = 1,
        .integral_gain = 0,
        .derivative_gain = 0
    };
    pid_controller_t pid_speed_right = pid_speed_left;


    while (1) {

        if(gpio_pressed) {
            gpio_pressed = false;
            // debugf("Press\n");

            bool res = nrf_write_data(1, tx_data, 3, true);
            debugf("Sending... %u\n", res);
        }

        // debugf("Left - [Speed: %u, Dir:%u]\r\n", encoder_left_get_value(), READ_BIT(TIM3->CR1, TIM_CR1_DIR)==TIM_CR1_DIR);
        // debugf("Right -[Speed: %u, Dir:%u]\r\n", encoder_right_get_value(), READ_BIT(TIM4->CR1, TIM_CR1_DIR)==TIM_CR1_DIR);
        // debugf("plot %d %d\n", encoder_left_get_value(),encoder_right_get_value());

        //! Basic ranging
        // debugf("VL53L0X Range...\n");
        // LL_mDelay(500);
        // sensors_VL53L0X_range_all(ranges);
        // debugf("F: %u, L: %u, R: %u, B: %u\n", ranges[0].rawDistance, ranges[1].rawDistance, ranges[2].rawDistance, ranges[3].rawDistance);
        
        // uint16_t range_value;
        // range_value = readRangeSingleMillimeters( &xTraStats );  // blocks until measurement is finished
        // if(range_value == false) { debugf("Error ranging with VL53L0X\n"); }
        // debugf("\n\nstatus = %X", xTraStats.rangeStatus);
        // debugf("\ndist = %u mm", xTraStats.rawDistance);
        // debugf("\nsignCnt = %07u MCPS", xTraStats.signalCnt);
        // debugf("\nambiCnt = %07u MCPS", xTraStats.ambientCnt);
        // debugf("\nspadCnt = %07u MCPS", xTraStats.spadCnt);
        // if(timeoutOccurred()) { debugf(" !!! Timeout !!! \n"); }
        // else { debugf("\n"); }

        //! Remote 
        // debugf("NRF status: [%02X]\n", nrf_get_status());

        uint32_t current_time = millis();

        nrf24l01_status_t * nrf_status = nrf_has_data_isr(); 
        if(nrf_status->rx_ready) {
            nrf_status->rx_ready = 0;
            nrf_read_data(nrf_data);

            int8_t forward_speed = (int8_t)nrf_data[0];
            int8_t steer_speed = (int8_t)nrf_data[1];

            target_speed_left = forward_speed + (int32_t)steer_speed / 3;
            target_speed_right = forward_speed - (int32_t)steer_speed / 3;
            debugf("NRF: %ld - %ld\n", target_speed_left, target_speed_right);

            SET_PIN(LD2_GPIO_Port, LD2_Pin, 1);
            scheduler_add_event(SCHEDULER_TASK_LED2, 50*MS, SCHEDULER_ONE_SHOT, blink_led2);

            scheduler_remove_event(SCHEDULER_TASK_LOST_CONNECTION);
            scheduler_add_event(SCHEDULER_TASK_LOST_CONNECTION, 200*MS, SCHEDULER_ONE_SHOT, lost_connection);
        }

        // if(current_time - motor_control_last_execution > MOTOR_CONTROL_INTERVAL) {
        //     motor_control_last_execution = current_time;

        //     //! Encoder
        //     int32_t current_left_speed = encoder_left_get_speed();
        //     int32_t current_right_speed = encoder_right_get_speed();
        //     debugf("ENCODER: %ld - %ld\n", current_left_speed, current_right_speed);


        //     //! PID
        //     int32_t motor_left_command = target_speed_left;
        //     int32_t motor_right_command = target_speed_right;
        //     // int32_t motor_left_command = pid_compute(&pid_speed_left, target_speed_left, current_left_speed);
        //     // int32_t motor_right_command = pid_compute(&pid_speed_right, target_speed_right, current_right_speed);
        //     debugf("PID OUT: %ld - %ld\n", motor_left_command, motor_right_command);

        //     //! Motors
        //     if(motor_left_command > 0) {
        //         motor_left_set_dir(1);
        //         motor_left_set_speed((uint16_t)motor_left_command);
        //     } 
        //     else {
        //         motor_left_set_dir(0);
        //         motor_left_set_speed((uint16_t)-motor_left_command);
        //     }
        //     if(motor_right_command > 0) {
        //         motor_right_set_dir(1);
        //         motor_right_set_speed((uint16_t)motor_right_command);
        //     } 
        //     else {
        //         motor_right_set_dir(0);
        //         motor_right_set_speed((uint16_t)-motor_right_command);
        //     }
        // }
    }
    return 0;
}


static void LL_Init(void) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    /* System interrupt init*/
    NVIC_SetPriority(MemoryManagement_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));  /* MemoryManagement_IRQn interrupt configuration */
    NVIC_SetPriority(BusFault_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));          /* BusFault_IRQn interrupt configuration */
    NVIC_SetPriority(UsageFault_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));        /* UsageFault_IRQn interrupt configuration */
    NVIC_SetPriority(SVCall_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));            /* SVCall_IRQn interrupt configuration */
    NVIC_SetPriority(DebugMonitor_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));      /* DebugMonitor_IRQn interrupt configuration */
    NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));            /* PendSV_IRQn interrupt configuration */
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));           /* SysTick_IRQn interrupt configuration */
}


void SystemClock_Config(void) {
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
    while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_0) { }
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);
    LL_RCC_HSE_Enable();

    /* Wait till HSE is ready */
    while(LL_RCC_HSE_IsReady() != 1) { }
    LL_RCC_PLL_ConfigDomain_48M(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_15, 144, LL_RCC_PLLQ_DIV_5);
    LL_RCC_PLL_Enable();

    /* Wait till PLL is ready */
    while(LL_RCC_PLL_IsReady() != 1) { }
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSE);
    /* Wait till System clock is ready */
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSE) { }
    LL_Init1msTick(25000000);
    LL_SetSystemCoreClock(25000000);

    /* Update the time base */
    // if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK) { Error_Handler(); }
    // LL_RCC_SetTIMPrescaler(LL_RCC_TIM_PRESCALER_TWICE);
}


// static uint32_t duty_cycle_percent = 0;
extern void UserButton_Callback(void) {
    gpio_pressed = true;

    // duty_cycle_percent += 10;
    // if(duty_cycle_percent > 100) { duty_cycle_percent = 0; } 
    // motor_right_set_speed(duty_cycle_percent);
    // motor_left_set_speed(duty_cycle_percent);
    // debugf("Press, new DC: %lu\n", duty_cycle_percent);
}


extern void _Error_Handler(char *file, int line) {
    printf("Error\n");
    while(1) {}
}


static uint32_t counter = 0;
extern uint32_t millis(void) {
    return counter;
}


void SysTick_Handler(void) {
    counter++;
}


#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) { 
    printf("Wrong parameters value: file %s on line %lu\r\n", file, line);
}
#endif
