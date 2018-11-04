#include "motor.h"
#include "gpio.h"


extern void TIM2_Motor_Init(void) {
  	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

  	//! Init GPIOs associated with the generation of PWM
	LL_GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = TIM2_CH1_Pin | TIM2_CH2_Pin | TIM2_CH3_Pin | TIM2_CH4_Pin;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
	LL_GPIO_Init(TIM2_CH1_GPIO_Port, &GPIO_InitStruct);

  	//! Init Timer2 for base generation of PWM
	LL_TIM_InitTypeDef LL_TIM_InitStruct;
	LL_TIM_InitStruct.Prescaler = __LL_TIM_CALC_PSC(SystemCoreClock, 8000000);
	LL_TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	LL_TIM_InitStruct.Autoreload = __LL_TIM_CALC_ARR(SystemCoreClock, LL_TIM_InitStruct.Prescaler, 15000);
	LL_TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	LL_TIM_InitStruct.RepetitionCounter = (uint8_t)0x00;
	LL_TIM_Init(TIM2, &LL_TIM_InitStruct);

	LL_TIM_EnableARRPreload(TIM2);

  	//! Init Timer OutPut Compare for generation of PWM
	LL_TIM_OC_InitTypeDef LL_TIM_OC_InitStruct;
	LL_TIM_OC_InitStruct.OCMode       = LL_TIM_OCMODE_PWM2;
	LL_TIM_OC_InitStruct.OCState      = LL_TIM_OCSTATE_ENABLE;
	LL_TIM_OC_InitStruct.OCNState     = LL_TIM_OCSTATE_ENABLE;
	LL_TIM_OC_InitStruct.CompareValue = 0;
	LL_TIM_OC_InitStruct.OCPolarity   = LL_TIM_OCPOLARITY_HIGH;
	LL_TIM_OC_InitStruct.OCNPolarity  = LL_TIM_OCPOLARITY_HIGH;
	LL_TIM_OC_InitStruct.OCIdleState  = LL_TIM_OCIDLESTATE_LOW;
	LL_TIM_OC_InitStruct.OCNIdleState = LL_TIM_OCIDLESTATE_LOW;
	LL_TIM_OC_Init(TIM2, LL_TIM_CHANNEL_CH1, &LL_TIM_OC_InitStruct);
  	LL_TIM_OC_EnablePreload(TIM2, LL_TIM_CHANNEL_CH1);
	LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH1);
	LL_TIM_OC_Init(TIM2, LL_TIM_CHANNEL_CH2, &LL_TIM_OC_InitStruct);
  	LL_TIM_OC_EnablePreload(TIM2, LL_TIM_CHANNEL_CH2);
	LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH2);
	LL_TIM_OC_Init(TIM2, LL_TIM_CHANNEL_CH3, &LL_TIM_OC_InitStruct);
  	LL_TIM_OC_EnablePreload(TIM2, LL_TIM_CHANNEL_CH3);
	LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH3);
	LL_TIM_OC_Init(TIM2, LL_TIM_CHANNEL_CH4, &LL_TIM_OC_InitStruct);
  	LL_TIM_OC_EnablePreload(TIM2, LL_TIM_CHANNEL_CH4);
	LL_TIM_CC_EnableChannel(TIM2, LL_TIM_CHANNEL_CH4);

  	motor_right_set_dir(MOTOR_DIR_FORWARD);
  	motor_left_set_dir(MOTOR_DIR_FORWARD);
  	motor_left_set_speed(0);
  	LL_TIM_EnableCounter(TIM2);			//! Enable counter
}


extern void motor_right_set_dir(uint8_t dir) {
	if(dir) {
		LL_GPIO_SetPinMode(TIM2_CH1_GPIO_Port, TIM2_CH1_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(TIM2_CH2_GPIO_Port, TIM2_CH2_Pin, LL_GPIO_MODE_ALTERNATE);

		LL_GPIO_SetOutputPin(TIM2_CH1_GPIO_Port, TIM2_CH1_Pin);
	}
	else {
		LL_GPIO_SetPinMode(TIM2_CH1_GPIO_Port, TIM2_CH1_Pin, LL_GPIO_MODE_ALTERNATE);
		LL_GPIO_SetPinMode(TIM2_CH2_GPIO_Port, TIM2_CH2_Pin, LL_GPIO_MODE_OUTPUT);

		LL_GPIO_SetOutputPin(TIM2_CH2_GPIO_Port, TIM2_CH2_Pin);
	}
}


extern void motor_left_set_dir(uint8_t dir) {
	if(dir) {
		LL_GPIO_SetPinMode(TIM2_CH3_GPIO_Port, TIM2_CH3_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(TIM2_CH4_GPIO_Port, TIM2_CH4_Pin, LL_GPIO_MODE_ALTERNATE);

		LL_GPIO_SetOutputPin(TIM2_CH3_GPIO_Port, TIM2_CH3_Pin);
	}
	else {
		LL_GPIO_SetPinMode(TIM2_CH3_GPIO_Port, TIM2_CH3_Pin, LL_GPIO_MODE_ALTERNATE);
		LL_GPIO_SetPinMode(TIM2_CH4_GPIO_Port, TIM2_CH4_Pin, LL_GPIO_MODE_OUTPUT);

		LL_GPIO_SetOutputPin(TIM2_CH4_GPIO_Port, TIM2_CH4_Pin);
	}
}


extern void motor_right_set_speed(uint32_t speed_percent) {
	//! speed = MAX*percent/100
	uint32_t period = (LL_TIM_GetAutoReload(TIM2) + 1);
	uint32_t speed = (uint32_t)((uint64_t)period * (uint64_t)speed_percent) / 100;
	LL_TIM_OC_SetCompareCH1(TIM2, speed);
	LL_TIM_OC_SetCompareCH2(TIM2, speed);
	printf("Set speed %lu/%lu\r\n", speed, period);
}

extern void motor_left_set_speed(uint32_t speed_percent) {
	//! speed = MAX*percent/100
	uint32_t period = (LL_TIM_GetAutoReload(TIM2) + 1);
	uint32_t speed = (uint32_t)((uint64_t)period * (uint64_t)speed_percent) / 100;
	LL_TIM_OC_SetCompareCH3(TIM2, speed);
	LL_TIM_OC_SetCompareCH4(TIM2, speed);
	printf("Set speed %lu/%lu\r\n", speed, period);
}