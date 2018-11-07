#define DEBUG_THIS_FILE		DEBUG_MAIN_FILE

#include "main.h"
#include "global.h"
#include "usart.h"
#include "gpio.h"
#include "encoder.h"
#include "motor.h"
#include "nrf24l01.h"
#include "scheduler.h"

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
}

extern int main(void) {
	LL_Init();
	SystemClock_Config();

	MX_GPIO_Init();
	MX_USART6_UART_Init();
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

	debugf("Init Done\r\n");

	// nrf_send(0xAA);

	while (1) {
		// printf("Left - [Speed: %u, Dir:%u]\r\n", encoder_left_get_value(), READ_BIT(TIM3->CR1, TIM_CR1_DIR)==TIM_CR1_DIR);
		// printf("Right -[Speed: %u, Dir:%u]\r\n", encoder_right_get_value(), READ_BIT(TIM4->CR1, TIM_CR1_DIR)==TIM_CR1_DIR);
		// printf("plot %d %d\n", encoder_left_get_value(),encoder_right_get_value());
		LL_mDelay(250);
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
}


// static uint32_t duty_cycle_percent = 0;
extern void UserButton_Callback(void) {
	// duty_cycle_percent += 10;
	// if(duty_cycle_percent > 100) { duty_cycle_percent = 0; } 
	// motor_right_set_speed(duty_cycle_percent);
	// motor_left_set_speed(duty_cycle_percent);
	// debug("Press, new DC: %lu\n", duty_cycle_percent);

	nrf_read_reg(0x00);
}


extern void _Error_Handler(char *file, int line) {
	while(1) {}
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) { 
	printf("Wrong parameters value: file %s on line %lu\r\n", file, line);
}
#endif
