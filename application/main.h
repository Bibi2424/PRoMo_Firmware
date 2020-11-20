#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_usart.h"
#include "stm32f4xx.h"
#include "stm32f4xx_ll_gpio.h"

//! NOTE: Go to BSP.h

//! User button
#define B1_Pin LL_GPIO_PIN_13
#define B1_GPIO_Port GPIOC

//! User LEDs
#define LD1_Pin LL_GPIO_PIN_1
#define LD1_GPIO_Port GPIOA
#define LD2_Pin LL_GPIO_PIN_8
#define LD2_GPIO_Port GPIOC

//! Programming pins
#define TMS_Pin LL_GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin LL_GPIO_PIN_14
#define TCK_GPIO_Port GPIOA

//! Encoders on Timer 3 and 4
#define TIM3_CH1_Pin LL_GPIO_PIN_4
#define TIM3_CH1_GPIO_Port GPIOB
#define TIM3_CH2_Pin LL_GPIO_PIN_5
#define TIM3_CH2_GPIO_Port GPIOB

#define TIM4_CH1_Pin LL_GPIO_PIN_6
#define TIM4_CH1_GPIO_Port GPIOB
#define TIM4_CH2_Pin LL_GPIO_PIN_7
#define TIM4_CH2_GPIO_Port GPIOB

//! Motor on Timer2
#define TIM2_CH1_Pin LL_GPIO_PIN_15
#define TIM2_CH1_GPIO_Port GPIOA
#define TIM2_CH2_Pin LL_GPIO_PIN_3
#define TIM2_CH2_GPIO_Port GPIOB
#define TIM2_CH3_Pin LL_GPIO_PIN_2
#define TIM2_CH3_GPIO_Port GPIOA
#define TIM2_CH4_Pin LL_GPIO_PIN_3
#define TIM2_CH4_GPIO_Port GPIOA

//! NRF24L01 on SPI2
#define NRF_CE_Pin LL_GPIO_PIN_10
#define NRF_CE_GPIO_Port GPIOB
#define NRF_CSN_Pin LL_GPIO_PIN_12
#define NRF_CSN_GPIO_Port GPIOB

#define NRF_SCK_Pin	LL_GPIO_PIN_13
#define NRF_SCK_GPIO_Port GPIOB
#define NRF_MISO_Pin LL_GPIO_PIN_14
#define NRF_MISO_GPIO_Port GPIOB
#define NRF_MOSI_Pin LL_GPIO_PIN_15
#define NRF_MOSI_GPIO_Port GPIOB

#define NRF_IRQ_Pin	LL_GPIO_PIN_2
#define NRF_IRQ_GPIO_Port GPIOB

//! FTDI on UART1

//! VL53l01 on I2C1

//! Aux UART6
#define USART_TX_Pin LL_GPIO_PIN_6
#define USART_TX_GPIO_Port GPIOC
#define USART_RX_Pin LL_GPIO_PIN_7
#define USART_RX_GPIO_Port GPIOC
#define USART_GPIO_Port GPIOC

//! MPU6050 and Aux I2C2

//! Aux SPI1


#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority, 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority, 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority, 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority, 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority, 0 bit  for subpriority */                                                                 
#endif

//! NOTE Comment/Uncomment the line below to expanse the "assert_param" macro in the HAL drivers code
#define USE_FULL_ASSERT    1U 

extern void UserButton_Callback(void);
extern void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

#endif
