#include <stdint.h>

#include "bsp.h"
#include "time.h"


volatile static uint32_t seconds_counter = 0;
volatile static uint32_t millis_counter = 0;


extern void time_init(void) {
    LL_TIM_InitTypeDef LL_TIM_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5);

    /* TIM5 interrupt Init */
    NVIC_SetPriority(TIM5_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 2, 0));
    NVIC_EnableIRQ(TIM5_IRQn);

    LL_TIM_InitStruct.Prescaler = __LL_TIM_CALC_PSC(SystemCoreClock / 2, 1000000);
    LL_TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    LL_TIM_InitStruct.Autoreload = 1000000;
    LL_TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_Init(TIM5, &LL_TIM_InitStruct);
    LL_TIM_DisableARRPreload(TIM5);
    LL_TIM_SetClockSource(TIM5, LL_TIM_CLOCKSOURCE_INTERNAL);
    LL_TIM_SetTriggerOutput(TIM5, LL_TIM_TRGO_RESET);
    LL_TIM_DisableMasterSlaveMode(TIM5);

    millis_counter = 0;
    LL_TIM_EnableCounter(TIM5);
    LL_TIM_ClearFlag_UPDATE(TIM5);
    LL_TIM_EnableIT_UPDATE(TIM5);
}


extern uint64_t get_time_absolute_us(void) {
    return seconds_counter * 1000000 + (uint32_t)LL_TIM_GetCounter(TIM5);
}


extern uint32_t get_time_microsecond(void) {
    return (uint32_t)LL_TIM_GetCounter(TIM5);
}

extern uint32_t get_time_millisecond(void) {
    return millis_counter;
}


void TIM5_IRQHandler(void) {
    if(LL_TIM_IsActiveFlag_UPDATE(TIM5) == 1) {
        LL_TIM_ClearFlag_UPDATE(TIM5);             // Clear the update interrupt flag
        seconds_counter++;
    }
}


void SysTick_Handler(void) {
    // TOGGLE_PIN(DEBUG_Pin_Port, DEBUG_Pin_1);
    millis_counter++;
}

