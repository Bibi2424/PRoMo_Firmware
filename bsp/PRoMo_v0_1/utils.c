#include "utils.h"


volatile static uint32_t counter = 0;


extern uint32_t millis(void) {
    return counter;
}


void SysTick_Handler(void) {
    // TOGGLE_PIN(DEBUG_Pin_Port, DEBUG_Pin_1);
    counter++;
}

