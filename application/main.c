#define DEBUG_THIS_FILE		DEBUG_MAIN_FILE

#include "main.h"
#include "global.h"
#include "usart.h"
#include "gpio.h"
#include "encoder.h"
#include "motor.h"
#include "i2c.h"
#include "nrf24l01.h"
#include "scheduler.h"
#include "VL53L0X.h"

static void LL_Init(void);
void SystemClock_Config(void);


/******************************************************************************/
/* IO Mapping on Nucleo 				                                   	  */
/* PA11/PA12, NA/NA - USART6 RX/TX				                           	  */
/* PA6/PA7, D11/D12 - RightEncoder A/B, TIM3 CH1/CH2                      	  */
/* PB6/PB7, D10/NA - LeftEncoder  A/B, TIM4 CH1/CH2                      	  */
/* PA0/P1, A0/A1 - Right Motor AIN1/AIN2, TIM2 CH1/CH2                     	  */
/* PA2/P3, D0/D1 - Right Motor BIN1/BIN2, TIM2 CH3/CH4                     	  */
/* PA5, D13 - LED 						                                   	  */
/* PC13, N/A - BP						                                   	  */
/******************************************************************************/
extern void blink_led(void) {
	LL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
	LL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
}

extern void range_one(void) {
	LL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
	LL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
}


static void lost_connection(void) {
	motor_left_set_speed(0);
	motor_right_set_speed(0);
	printf("Lost Connection with Remote\r\n");
}


extern int main(void) {
	LL_Init();
	SystemClock_Config();

	MX_GPIO_Init();
	#if(DEBUG_UART == 1)
		MX_USART1_UART_Init(DEBUG_BAUDRATE);
	#elif(DEBUG_UART == 6)
		MX_USART6_UART_Init(DEBUG_BAUDRATE);
	#endif
	setbuf(stdout, NULL); 		//! For unbuffered ouput
	debugf("\r\n**************************************\r\n");
	debugf(    "* Boot..\r\n");
	debugf(    "* System_Frequency: %lu  MHz\n", SystemCoreClock);
	debugf(    "**************************************\r\n");

	TIM34_Encoder_Init();
	TIM2_Motor_Init();

	SPI2_NRF24L01_Init();

	scheduler_init();
	scheduler_add_event(0, 1*SECOND, SCHEDULER_ALWAYS, blink_led);
	// scheduler_add_event(1, 100*MS, SCHEDULER_ONE, blink_led);
	// scheduler_add_event(1, 500*MS, SCHEDULER_ALWAYS, range_one);

	i2c1_init();
	bool res = initVL53L0X(1);
	if(res == false) { printf("Error Init VL53L0X\n"); }
	res = setMeasurementTimingBudget( 50 * 1000UL );		// integrate over 500 ms per measurement
	if(res == false) { printf("Error setring VL53L0X\n"); }

	debugf("Init Done\r\n");

	// nrf_send(0xAA);

	uint8_t nrf_rx_size = 0;
	uint8_t nrf_data[32] = {0};

	statInfo_t xTraStats;

	while (1) {
		// printf("Left - [Speed: %u, Dir:%u]\r\n", encoder_left_get_value(), READ_BIT(TIM3->CR1, TIM_CR1_DIR)==TIM_CR1_DIR);
		// printf("Right -[Speed: %u, Dir:%u]\r\n", encoder_right_get_value(), READ_BIT(TIM4->CR1, TIM_CR1_DIR)==TIM_CR1_DIR);
		// printf("plot %d %d\n", encoder_left_get_value(),encoder_right_get_value());

		//! Basic ranging
		printf("VL53L0X Range...\n");
		LL_mDelay(500);
		uint16_t range_value;
		range_value = readRangeSingleMillimeters( &xTraStats );	// blocks until measurement is finished
		if(range_value == false) { printf("Error ranging with VL53L0X\n"); }
		debugf("\n\nstatus = %X", xTraStats.rangeStatus);
		debugf("\ndist = %u mm", xTraStats.rawDistance);
		debugf("\nsignCnt = %07u MCPS", xTraStats.signalCnt);
		debugf("\nambiCnt = %07u MCPS", xTraStats.ambientCnt);
		debugf("\nspadCnt = %07u MCPS", xTraStats.spadCnt);
		if(timeoutOccurred()) { debugf(" !!! Timeout !!! \n"); }
		else { debugf("\n"); }

		//! Remote to motor
		// LL_mDelay(50);
		// nrf_rx_size = nrf_read_data(nrf_data);
		// if(nrf_rx_size) {
		// 	scheduler_add_event(1, 200*MS, SCHEDULER_ONE_SHOT, lost_connection);

		// 	printf("NRF RX (%u): [", nrf_rx_size);
		// 	for(uint8_t i = 0; i < nrf_rx_size; i++) { printf("%02X:", nrf_data[i]); }
		// 	printf("]\r\n");

		// 	int8_t forward_speed = (int8_t)nrf_data[0];
		// 	int8_t steer_speed = (int8_t)nrf_data[1];

		// 	int8_t left_speed = forward_speed + steer_speed;
		// 	int8_t right_speed = forward_speed - steer_speed;


		// 	if(left_speed > 0) {
		// 		motor_left_set_dir(1);
		// 		motor_left_set_speed((uint16_t)left_speed);
		// 	} 
		// 	else {
		// 		motor_left_set_dir(0);
		// 		motor_left_set_speed((uint16_t)-left_speed);
		// 	}
		// 	if(right_speed > 0) {
		// 		motor_right_set_dir(1);
		// 		motor_right_set_speed((uint16_t)right_speed);
		// 	} 
		// 	else {
		// 		motor_right_set_dir(0);
		// 		motor_right_set_speed((uint16_t)-right_speed);
		// 	}
		// }
		// else {
		// 	printf(".\n");
		// }

		//! Basic blink
		// printf("Status: %02X\r\n", nrf_get_status());
		// LL_GPIO_SetOutputPin(LD2_GPIO_Port, LD2_Pin);
		// LL_mDelay(250);
		// LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);

	}
	return 0;
}


static void LL_Init(void) {
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	/* System interrupt init*/
	NVIC_SetPriority(MemoryManagement_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));	/* MemoryManagement_IRQn interrupt configuration */
	NVIC_SetPriority(BusFault_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));			/* BusFault_IRQn interrupt configuration */
	NVIC_SetPriority(UsageFault_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));		/* UsageFault_IRQn interrupt configuration */
	NVIC_SetPriority(SVCall_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));			/* SVCall_IRQn interrupt configuration */
	NVIC_SetPriority(DebugMonitor_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));		/* DebugMonitor_IRQn interrupt configuration */
	NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));			/* PendSV_IRQn interrupt configuration */
	NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));			/* SysTick_IRQn interrupt configuration */
}


void SystemClock_Config(void) {
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
	if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2) {
		Error_Handler();  
	}
	LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
	LL_RCC_HSI_SetCalibTrimming(16);
	LL_RCC_HSI_Enable();
	/* Wait till HSI is ready */
	while(LL_RCC_HSI_IsReady() != 1) {}
	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_16, 336, LL_RCC_PLLP_DIV_4);
	LL_RCC_PLL_Enable();
	/* Wait till PLL is ready */
	while(LL_RCC_PLL_IsReady() != 1) {}
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
	 /* Wait till System clock is ready */
	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {}
	LL_Init1msTick(84000000);
	LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
	LL_SetSystemCoreClock(84000000);
	LL_RCC_SetTIMPrescaler(LL_RCC_TIM_PRESCALER_TWICE);
	/* SysTick_IRQn interrupt configuration */
	NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));

	SysTick_Config(SystemCoreClock / 1000);
}


// static uint32_t duty_cycle_percent = 0;
extern void UserButton_Callback(void) {
	printf("Press\n");
	// duty_cycle_percent += 10;
	// if(duty_cycle_percent > 100) { duty_cycle_percent = 0; } 
	// motor_right_set_speed(duty_cycle_percent);
	// motor_left_set_speed(duty_cycle_percent);
	// debug("Press, new DC: %lu\n", duty_cycle_percent);

	// nrf_read_register(0x00);
	
	// SPI2_NRF24L01_Init();
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
