#include "encoder.h"
#include "gpio.h"

extern void TIM3_Encoder_Init(void) {

  	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
  	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);

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

	LL_TIM_InitTypeDef LL_TIM_InitStruct;
	LL_TIM_InitStruct.Prescaler = 0;
	LL_TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
	LL_TIM_InitStruct.Autoreload = 0xFFFF;
	LL_TIM_InitStruct.ClockDivision = 0;
	LL_TIM_InitStruct.RepetitionCounter = 0;
	LL_TIM_Init(TIM3, &LL_TIM_InitStruct);
	LL_TIM_Init(TIM4, &LL_TIM_InitStruct);

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

	// NVIC_SetPriority(TIM3_IRQn, 0);
	// NVIC_EnableIRQ(TIM3_IRQn);			//! Configure the NVIC to handle TIM3 capture/compare interrupt
	// LL_TIM_EnableIT_CC1(TIM3);			//! Enable the capture/compare interrupt for channel 1
	LL_TIM_EnableCounter(TIM3);			//! Enable counter
	LL_TIM_EnableCounter(TIM4);			//! Enable counter
}


extern uint16_t encoder_right_get_value(void) {
	return LL_TIM_ReadReg(TIM3, CCR1);
}


extern uint16_t encoder_left_get_value(void) {
	return LL_TIM_ReadReg(TIM4, CCR1);
}