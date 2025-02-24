#include <stddef.h>

#include "gpio.h"
#include "time.h"


static gpio_interrupt_cb gpio_int_cb = NULL;


void MX_GPIO_Init(void) {

	LL_EXTI_InitTypeDef EXTI_InitStruct;
	LL_GPIO_InitTypeDef GPIO_InitStruct;
	/* GPIO Ports Clock Enable */
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);

	// LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin);

	//! BP1
	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTA, LL_SYSCFG_EXTI_LINE0);

	EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_0;
	EXTI_InitStruct.LineCommand = ENABLE;
	EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
	EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
	LL_EXTI_Init(&EXTI_InitStruct);

	LL_GPIO_SetPinPull(B1_GPIO_Port, B1_Pin, LL_GPIO_PULL_NO);
	LL_GPIO_SetPinMode(B1_GPIO_Port, B1_Pin, LL_GPIO_MODE_INPUT);

	NVIC_SetPriority(EXTI0_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 8, 0));
	NVIC_EnableIRQ(EXTI0_IRQn); 

	//! LD1
	LL_GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.Pin = LD1_Pin;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(LD1_GPIO_Port, &GPIO_InitStruct);
	LL_GPIO_SetOutputPin(LD1_GPIO_Port, LD1_Pin);

	//! LD2
	LL_GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.Pin = LD2_Pin;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);
	LL_GPIO_SetOutputPin(LD2_GPIO_Port, LD2_Pin);

	//! Debug Pin
	LL_GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.Pin = DEBUG_Pin_1 | DEBUG_Pin_2;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(DEBUG_Pin_Port, &GPIO_InitStruct);
	SET_PIN(DEBUG_Pin_Port, DEBUG_Pin_1, GPIO_LOW);
	SET_PIN(DEBUG_Pin_Port, DEBUG_Pin_2, GPIO_LOW);
}

//! TODO: Expand
void gpio__register_callback(gpio_interrupt_cb cb) {
	gpio_int_cb = cb;
}


void EXTI0_IRQHandler(void)
{
	/* Manage Flags */
	if(LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_0) != RESET)
	{
		static uint32_t last_irq_time = 0;
		if(millis() - last_irq_time > 10) {
			// UserButton_Callback(GPIO_INTERRUPT_FALLING);
			if(gpio_int_cb != NULL) { gpio_int_cb(GPIO_INTERRUPT_FALLING); }
			last_irq_time = millis();
		}
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_0);
	}
}
