#include "ws2812b.h"


#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_gpio.h"



#define SET_PIN(PORT, PIN, VALUE)	do { if(VALUE == 0) { LL_GPIO_ResetOutputPin(PORT, PIN); } else { LL_GPIO_SetOutputPin(PORT, PIN); } } while(0)
#define TOGGLE_PIN(PORT, PIN)		LL_GPIO_TogglePin(PORT, PIN)

#define PIN_HIGH	LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_12)
#define PIN_LOW		LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_12)
#define PIN_TOGGLE	LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_12)

extern void ws2812b_init(void) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM10);

	LL_GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = LL_GPIO_PIN_12;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	// LL_GPIO_ResetOutputPin(GPIOC, LD1_Pin);
	PIN_LOW;

    //! Init Timer10 for precision delay
    LL_TIM_InitTypeDef LL_TIM_InitStruct;
    //! NOTE: 1tick = 0.1us
    LL_TIM_InitStruct.Prescaler = __LL_TIM_CALC_PSC(SystemCoreClock, 10000000);
    // printf("[%u/%u]\n", SystemCoreClock, __LL_TIM_CALC_PSC(SystemCoreClock, 10000000));
    LL_TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    LL_TIM_InitStruct.Autoreload = 0xffff; //! This will be set by the lib
    LL_TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_InitStruct.RepetitionCounter = (uint8_t)0x00;
    LL_TIM_Init(TIM10, &LL_TIM_InitStruct);

    LL_TIM_EnableCounter(TIM10);
}


static void delay_ticks(uint32_t ticks) {
    LL_TIM_SetCounter(TIM10, 0);
    // LL_mDelay(50);
    while(LL_TIM_GetCounter(TIM10) < ticks);
}


extern void ws2812b_send(rgb_t *strip, uint16_t size) {
	__disable_irq();
	// for(uint16_t i = 0; i < 100; i++) {
	// 	if(i % 2 == 0) {
	// 		PIN_HIGH;
	// 		delay_ticks(20);
	// 	}
	// 	else {
	// 		PIN_LOW;
	// 		delay_ticks(10);
	// 	}
	// }
	for(uint16_t i = 0; i < size; i++) {
		//! Green
		for(int8_t j = 7; j >= 0; j--) {
			bool bit = (strip[i].g >> j) & 0x01;
			PIN_HIGH;
			bit ? delay_ticks(4): delay_ticks(0);
			PIN_LOW;
			bit ? delay_ticks(0): delay_ticks(4);
		}

		//! Red
		for(int8_t j = 7; j >= 0; j--) {
			bool bit = (strip[i].r >> j) & 0x01;
			PIN_HIGH;
			bit ? delay_ticks(4): delay_ticks(0);
			PIN_LOW;
			bit ? delay_ticks(0): delay_ticks(4);
		}

		//! Blue
		for(int8_t j = 7; j >= 0; j--) {
			bool bit = (strip[i].b >> j) & 0x01;
			PIN_HIGH;
			bit ? delay_ticks(4): delay_ticks(0);
			PIN_LOW;
			bit ? delay_ticks(0): delay_ticks(4);
		}
	}

	__enable_irq();

	delay_ticks(600);
}