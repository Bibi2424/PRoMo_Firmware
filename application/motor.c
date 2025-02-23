#define DEBUG_THIS_FILE	DEBUG_MOTOR_FILE

#include <stdint.h>
#include <stdbool.h>

#include "utils.h"
#include "debug.h"

#include "motor.h"
#include "gpio.h"


static uint32_t max_autoreload = 0;


extern void motors_init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct;

  	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

  	//! Init GPIOs associated with the generation of PWM
  	LL_GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.Pin = TIM2_CH1_Pin /*| TIM2_CH2_Pin*/ | TIM2_CH3_Pin | TIM2_CH4_Pin;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	// GPIO_InitStruct.Alternate = LL_GPIO_AF_1;
	LL_GPIO_Init(TIM2_CH1_GPIO_Port, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = TIM2_CH2_Pin;
	LL_GPIO_Init(TIM2_CH2_GPIO_Port, &GPIO_InitStruct);

	LL_GPIO_SetAFPin_8_15(TIM2_CH1_GPIO_Port, TIM2_CH1_Pin, LL_GPIO_AF_1);
	LL_GPIO_SetAFPin_0_7(TIM2_CH2_GPIO_Port, TIM2_CH2_Pin, LL_GPIO_AF_1);
	LL_GPIO_SetAFPin_0_7(TIM2_CH3_GPIO_Port, TIM2_CH3_Pin, LL_GPIO_AF_1);
	LL_GPIO_SetAFPin_0_7(TIM2_CH4_GPIO_Port, TIM2_CH4_Pin, LL_GPIO_AF_1);

	//! Motor Driver Enable Pin
  	LL_GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.Pin = MOTOR_ENABLE_Pin;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(MOTOR_ENABLE_Pin_Port, &GPIO_InitStruct);
  	SET_PIN(MOTOR_ENABLE_Pin_Port, MOTOR_ENABLE_Pin, 1);

  	//! Init Timer2 for base generation of PWM
	LL_TIM_InitTypeDef LL_TIM_InitStruct;
	LL_TIM_InitStruct.Prescaler = __LL_TIM_CALC_PSC(SystemCoreClock / 2, 1000000);
	LL_TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	//! NOTE: 25kHz PWM
	LL_TIM_InitStruct.Autoreload = __LL_TIM_CALC_ARR(SystemCoreClock / 2, LL_TIM_InitStruct.Prescaler, 25000);
	LL_TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
	LL_TIM_InitStruct.RepetitionCounter = (uint8_t)0x00;

	max_autoreload = LL_TIM_InitStruct.Autoreload + 1;

	LL_TIM_Init(TIM2, &LL_TIM_InitStruct);
	LL_TIM_EnableARRPreload(TIM2);

  	//! Init Timer Output Compare for generation of PWM
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

  	motor_set_dir(RIGHT_SIDE, MOTOR_DIR_DISABLE);
  	motor_set_speed(RIGHT_SIDE, 0);
  	motor_set_dir(LEFT_SIDE, MOTOR_DIR_DISABLE);
  	motor_set_speed(LEFT_SIDE, 0);
  	LL_TIM_EnableCounter(TIM2);			//! Enable counter
}


static void motor_left_set_dir(uint8_t dir) {
#if INVERSE_MOTOR_G
	dir = dir ^ 1;
#endif
	if(dir == MOTOR_DIR_FORWARD) {
		LL_GPIO_ResetOutputPin(TIM2_CH1_GPIO_Port, TIM2_CH1_Pin);
		LL_GPIO_ResetOutputPin(TIM2_CH2_GPIO_Port, TIM2_CH2_Pin);
		LL_GPIO_SetPinMode(TIM2_CH1_GPIO_Port, TIM2_CH1_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(TIM2_CH2_GPIO_Port, TIM2_CH2_Pin, LL_GPIO_MODE_ALTERNATE);

		LL_GPIO_SetOutputPin(TIM2_CH1_GPIO_Port, TIM2_CH1_Pin);
	}
	else if(dir == MOTOR_DIR_REVERSE) {
		LL_GPIO_ResetOutputPin(TIM2_CH1_GPIO_Port, TIM2_CH1_Pin);
		LL_GPIO_ResetOutputPin(TIM2_CH2_GPIO_Port, TIM2_CH2_Pin);
		LL_GPIO_SetPinMode(TIM2_CH1_GPIO_Port, TIM2_CH1_Pin, LL_GPIO_MODE_ALTERNATE);
		LL_GPIO_SetPinMode(TIM2_CH2_GPIO_Port, TIM2_CH2_Pin, LL_GPIO_MODE_OUTPUT);

		LL_GPIO_SetOutputPin(TIM2_CH2_GPIO_Port, TIM2_CH2_Pin);
	}
	else {
		LL_GPIO_SetPinMode(TIM2_CH1_GPIO_Port, TIM2_CH1_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(TIM2_CH2_GPIO_Port, TIM2_CH2_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_ResetOutputPin(TIM2_CH1_GPIO_Port, TIM2_CH1_Pin);
		LL_GPIO_ResetOutputPin(TIM2_CH2_GPIO_Port, TIM2_CH2_Pin);
	}
}


static void motor_right_set_dir(uint8_t dir) {
#if INVERSE_MOTOR_R
	dir = dir ^ 1;
#endif
	if(dir == MOTOR_DIR_FORWARD) {
		LL_GPIO_ResetOutputPin(TIM2_CH3_GPIO_Port, TIM2_CH3_Pin);
		LL_GPIO_ResetOutputPin(TIM2_CH4_GPIO_Port, TIM2_CH4_Pin);
		LL_GPIO_SetPinMode(TIM2_CH3_GPIO_Port, TIM2_CH3_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(TIM2_CH4_GPIO_Port, TIM2_CH4_Pin, LL_GPIO_MODE_ALTERNATE);

		LL_GPIO_SetOutputPin(TIM2_CH3_GPIO_Port, TIM2_CH3_Pin);
	}
	else if(dir == MOTOR_DIR_REVERSE) {
		LL_GPIO_ResetOutputPin(TIM2_CH3_GPIO_Port, TIM2_CH3_Pin);
		LL_GPIO_ResetOutputPin(TIM2_CH4_GPIO_Port, TIM2_CH4_Pin);
		LL_GPIO_SetPinMode(TIM2_CH3_GPIO_Port, TIM2_CH3_Pin, LL_GPIO_MODE_ALTERNATE);
		LL_GPIO_SetPinMode(TIM2_CH4_GPIO_Port, TIM2_CH4_Pin, LL_GPIO_MODE_OUTPUT);

		LL_GPIO_SetOutputPin(TIM2_CH4_GPIO_Port, TIM2_CH4_Pin);
	}
	else {
		LL_GPIO_SetPinMode(TIM2_CH3_GPIO_Port, TIM2_CH3_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_SetPinMode(TIM2_CH4_GPIO_Port, TIM2_CH4_Pin, LL_GPIO_MODE_OUTPUT);
		LL_GPIO_ResetOutputPin(TIM2_CH3_GPIO_Port, TIM2_CH3_Pin);
		LL_GPIO_ResetOutputPin(TIM2_CH4_GPIO_Port, TIM2_CH4_Pin);
	}
}


extern void motor_set_dir(actuator_t side, uint8_t dir) {
	if(side == LEFT_SIDE) {
		motor_left_set_dir(dir);
	}
	else if(side == RIGHT_SIDE) {
		motor_right_set_dir(dir);
	}
}


static void motor_left_set_speed(uint32_t percent) {
	uint32_t command = (uint32_t) (max_autoreload * percent / 100UL);
	if(command > max_autoreload) { command = max_autoreload; }

	LL_TIM_OC_SetCompareCH1(TIM2, command);
	LL_TIM_OC_SetCompareCH2(TIM2, command);
	// debugf("Left command %0.2f -> %lu/%lu\r\n", percent, command, max_autoreload);
}


static void motor_right_set_speed(uint32_t percent) {
	uint32_t command = (uint32_t) (max_autoreload * percent / 100UL);
	if(command > max_autoreload) { command = max_autoreload; }

	LL_TIM_OC_SetCompareCH3(TIM2, command);
	LL_TIM_OC_SetCompareCH4(TIM2, command);
	// debugf("Right command %0.2f -> %lu/%lu\r\n", percent, command, max_autoreload);
}


extern void motor_set_speed(actuator_t side, uint32_t percent) {
	if(side == LEFT_SIDE || side == BOTH_SIDE) {
		motor_left_set_speed(percent);
	}
	if(side == RIGHT_SIDE || side == BOTH_SIDE) {
		motor_right_set_speed(percent);
	}
}
