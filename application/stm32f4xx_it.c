#include "stm32f4xx.h"
#include "stm32f4xx_it.h"



/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/******************************************************************************/
/*            Cortex-M4 Processor Interruption and Exception Handlers         */ 
/******************************************************************************/
void NMI_Handler(void) {

}


void HardFault_Handler(void) {

	while (1) {
	}
}


void MemManage_Handler(void) {

	while (1) {
	}
}


void BusFault_Handler(void) {

	while (1) {
	}
}


void UsageFault_Handler(void) {

	while (1) {
	}
}


void SVC_Handler(void) {

}


void DebugMon_Handler(void) {

}


void PendSV_Handler(void) {

}


void SysTick_Handler(void) {

}
