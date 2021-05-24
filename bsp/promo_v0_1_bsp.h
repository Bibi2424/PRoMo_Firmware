#ifndef PROMO_V0_1_BSP_H
#define PROMO_V0_1_BSP_H


//! User button
#define B1_Pin LL_GPIO_PIN_0
#define B1_GPIO_Port GPIOA

//! User LEDs
#define LD1_Pin LL_GPIO_PIN_1
#define LD1_GPIO_Port GPIOA
#define LD2_Pin LL_GPIO_PIN_8
#define LD2_GPIO_Port GPIOC

//! User LEDs
#define DEBUG_Pin_1 LL_GPIO_PIN_0
#define DEBUG_Pin_2 LL_GPIO_PIN_1
#define DEBUG_Pin_Port GPIOB

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

//! Motor enable on PC4
#define MOTOR_ENABLE_Pin LL_GPIO_PIN_4
#define MOTOR_ENABLE_Pin_Port GPIOC

//! NRF24L01 on SPI2
#define NRF_CE_Pin LL_GPIO_PIN_10
#define NRF_CE_GPIO_Port GPIOB
#define NRF_CSN_Pin LL_GPIO_PIN_12
#define NRF_CSN_GPIO_Port GPIOB
#define NRF_IRQ_Pin LL_GPIO_PIN_2
#define NRF_IRQ_GPIO_Port GPIOB

#define NRF_SCK_Pin	LL_GPIO_PIN_13
#define NRF_SCK_GPIO_Port GPIOB
#define NRF_MISO_Pin LL_GPIO_PIN_14
#define NRF_MISO_GPIO_Port GPIOB
#define NRF_MOSI_Pin LL_GPIO_PIN_15
#define NRF_MOSI_GPIO_Port GPIOB

#define NRF_IRQ_Pin	LL_GPIO_PIN_2
#define NRF_IRQ_GPIO_Port GPIOB

//! VL53l01 on I2C1
#define VL53L0X_I2C_Port	GPIOB
#define VL53L0X_I2C_SCL		LL_GPIO_PIN_8
#define VL53L0X_I2C_SDA		LL_GPIO_PIN_9
#define VL53L0X_XSHUT_Port	GPIOC
#define VL53L0X_XSHUT1_Pin	LL_GPIO_PIN_0
#define VL53L0X_XSHUT2_Pin	LL_GPIO_PIN_1
#define VL53L0X_XSHUT3_Pin	LL_GPIO_PIN_2
#define VL53L0X_XSHUT4_Pin	LL_GPIO_PIN_3

//! MPU6050 and Aux I2C2

//! Aux SPI1

//! FTDI on UART1
#define USART1_TX_Pin LL_GPIO_PIN_9
#define USART1_TX_GPIO_Port GPIOA
#define USART1_RX_Pin LL_GPIO_PIN_10
#define USART1_RX_GPIO_Port GPIOA
#define USART1_CTS_Pin LL_GPIO_PIN_11
#define USART1_CTS_GPIO_Port GPIOA
#define USART1_RTSPin LL_GPIO_PIN_12
#define USART1_RTSGPIO_Port GPIOA
#define USART1_GPIO_Port GPIOA

//! Aux UART6
#define USART6_TX_Pin LL_GPIO_PIN_6
#define USART6_TX_GPIO_Port GPIOC
#define USART6_RX_Pin LL_GPIO_PIN_7
#define USART6_RX_GPIO_Port GPIOC
#define USART6_GPIO_Port GPIOC


#endif