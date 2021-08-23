#define DEBUG_THIS_FILE	DEBUG_ENCODER_FILE

#include <stdint.h>
#include <stdbool.h>

#include "global.h"
#include "debug.h"

#include "encoder.h"
#include "gpio.h"



static int16_t last_value_left = 0, last_value_right = 0;


extern void encoders_init(void) {

  	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
  	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);

  	//! GPIO Init
	LL_GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = TIM3_CH1_Pin | TIM3_CH2_Pin;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_2;
	LL_GPIO_Init(TIM3_CH1_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = TIM4_CH1_Pin | TIM4_CH2_Pin;
	LL_GPIO_Init(TIM4_CH1_GPIO_Port, &GPIO_InitStruct);

	//! Timer Init
	LL_TIM_InitTypeDef LL_TIM_InitStruct;
	LL_TIM_InitStruct.Prescaler = 0;
	LL_TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	LL_TIM_InitStruct.Autoreload = 0xFFFF;
	LL_TIM_InitStruct.ClockDivision = 0;
	LL_TIM_InitStruct.RepetitionCounter = 0;
	LL_TIM_Init(TIM3, &LL_TIM_InitStruct);
	LL_TIM_Init(TIM4, &LL_TIM_InitStruct);

	//! Configuring Quadrature decoder
	LL_TIM_ENCODER_InitTypeDef TIM_EncoderInitStruct;
	TIM_EncoderInitStruct.EncoderMode = LL_TIM_ENCODERMODE_X4_TI12;
	TIM_EncoderInitStruct.IC1Polarity = LL_TIM_IC_POLARITY_RISING;
	TIM_EncoderInitStruct.IC1ActiveInput = LL_TIM_ACTIVEINPUT_DIRECTTI;
	TIM_EncoderInitStruct.IC1Prescaler = LL_TIM_ICPSC_DIV1;
	TIM_EncoderInitStruct.IC1Filter = LL_TIM_IC_FILTER_FDIV1;
	TIM_EncoderInitStruct.IC2Polarity = LL_TIM_IC_POLARITY_RISING;
	TIM_EncoderInitStruct.IC2ActiveInput = LL_TIM_ACTIVEINPUT_DIRECTTI;
	TIM_EncoderInitStruct.IC2Prescaler = LL_TIM_ICPSC_DIV1;
	TIM_EncoderInitStruct.IC2Filter = LL_TIM_IC_FILTER_FDIV1;
	LL_TIM_ENCODER_Init(TIM3, &TIM_EncoderInitStruct);
	LL_TIM_ENCODER_Init(TIM4, &TIM_EncoderInitStruct);

	// NVIC_SetPriority(TIM3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 6, 0));
	// NVIC_EnableIRQ(TIM3_IRQn);			//! Configure the NVIC to handle TIM3 capture/compare interrupt
	// LL_TIM_EnableIT_CC1(TIM3);			//! Enable the capture/compare interrupt for channel 1
	LL_TIM_EnableCounter(TIM3);			//! Enable counter
	LL_TIM_EnableCounter(TIM4);			//! Enable counter

	//! NOTE: Should be 0 on init be leave it as a precaution
	last_value_left = encoder_get_value(LEFT_SIDE);
	last_value_right = encoder_get_value(RIGHT_SIDE);
}


extern uint32_t encoder_get_value(actuator_t side) {
	if(side == LEFT_SIDE) {
		return LL_TIM_ReadReg(TIM3, CCR1);
	}
	else if(side == RIGHT_SIDE) {
		return LL_TIM_ReadReg(TIM4, CCR1);
	}
	return 0;
}


static /*inline*/ int32_t calibrate_speed(int32_t tick_speed, uint32_t elapse_time_ms) {
	//! NOTE: 1/T * speed_ticks * wheel_radius * (PI / TICKS_PER_TURN)
	int32_t speed_mms = ((1000 / (int32_t)elapse_time_ms) * tick_speed * WHEEL_RADIUS_MM / TICKS_PER_WHEEL_TURN_DIV_PI);
	//! NOTE: speed_mms = [-MAX_SPEED/s..MAX_SPEED/s]
	return speed_mms;
}


extern int32_t encoder_get_speed(actuator_t side) {
	static uint32_t last_compute_time_left = 0, last_compute_time_right = 0;

	int32_t speed = 0;
	uint32_t now_ms = millis();

	if(side == LEFT_SIDE) {
		int16_t current_value = encoder_get_value(LEFT_SIDE);
		//! Need to compute that diff from int16_t to not get errors, later moving to int32_t for avoiding overflow during calibration
		int16_t speed16 = current_value - last_value_left;
		speed = (int32_t)speed16;
		#if INVERSE_LEFT_ENCODER
			speed = -speed;
		#endif
		last_value_left = current_value;

		speed = calibrate_speed(speed, now_ms - last_compute_time_left);
		last_compute_time_left = now_ms;

	}
	else if(side == RIGHT_SIDE) {
		int16_t current_value = encoder_get_value(RIGHT_SIDE);
		//! Need to compute that diff from int16_t to not get errors, later moving to int32_t for avoiding overflow during calibration
		int16_t speed16 = current_value - last_value_right;
		speed = (int32_t)speed16;
		#if INVERSE_RIGHT_ENCODER
			speed = -speed;
		#endif
		last_value_right = current_value;

		speed = calibrate_speed(speed, now_ms - last_compute_time_right);
		last_compute_time_right = now_ms;
	}

	return speed;
}
