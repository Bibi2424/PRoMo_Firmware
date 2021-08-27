#include "ws2812b.h"

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_spi.h"


//-----------------------------------------------------------------------------
//! TODO: Integrate with bsp
//! TODO: Change SPI encoding for a LUT
//! TODO: Use DMA to reload SPI to have a non blocking send command
//! TODO: Improve timings for dummy implementation
//! TODO: Try a Timer in Output compare (then with DMA)
//-----------------------------------------------------------------------------


// #define DATA_PIN	LL_GPIO_PIN_12
// #define DATA_PORT	GPIOC
#define DATA_PIN	LL_GPIO_PIN_2
#define DATA_PORT	GPIOD


#if WS2812B_DRIVE_METHOD == USE_SPI_IMPLEMENTATION

extern void ws2812b_init(void) {
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);

	LL_GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = DATA_PIN | LL_GPIO_PIN_10;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_6;
	LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	LL_SPI_InitTypeDef SPI_InitStruct = {0};
	SPI_InitStruct.TransferDirection = LL_SPI_FULL_DUPLEX;
	SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
	SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
	SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_LOW;
	SPI_InitStruct.ClockPhase = LL_SPI_PHASE_2EDGE;
	SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
	//! NOTE: SPI1 - APB2, SPI2/SPI3 - APB1
	SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV8;	//! SPI3: 3.75MHz -> 0.267us
	// SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV16;	//! SPI3: 1.875MHz
	SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
	SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
	SPI_InitStruct.CRCPoly = 10;
	LL_SPI_Init(SPI3, &SPI_InitStruct);
	LL_SPI_SetStandard(SPI3, LL_SPI_PROTOCOL_MOTOROLA);

	LL_SPI_Enable(SPI3);
}


static void spi_send_multiple_bytes_waiting(SPI_TypeDef *SPIx, uint8_t* write_data, uint8_t size) {
	for(uint8_t i = 0; i < size; i++) {
		LL_SPI_TransmitData8(SPIx, write_data[i]);
		//! NOTE: Only wait for the tx_buffer to being copied into the shift register
		while(LL_SPI_IsActiveFlag_TXE(SPIx) == 0);
	}
}

//! NOTE: 3bits encoding, need 1bit ~= 0.4us
// static void encode_byte(uint8_t in, uint8_t* out) {
// 	uint32_t temp = 0;
// 	for(uint8_t i = 0; i < 8; i++) {
// 		temp <<= 3;
// 		if(in & 0x80) { temp |= 0b110; }
// 		else { temp |= 0b100; }
// 		in <<= 1;
// 	}
// 	*out = (temp >> 16) & 0xff;
// 	*(out + 1) = (temp >> 8) & 0xff;
// 	*(out + 2) = temp & 0xff;
// }


// extern void ws2812b_send(rgb_t *strip, uint16_t size) {

// 	uint8_t buffer[100];
// 	for(uint16_t i = 0; i < size; i++) {
// 		encode_byte(strip[i].g, &buffer[9*i]);
// 		encode_byte(strip[i].r, &buffer[9*i+3]);
// 		encode_byte(strip[i].b, &buffer[9*i+6]);
// 	}

// 	spi_send_multiple_bytes_waiting(SPI3, buffer, 9*size);
// }


//! NOTE: 4bits encoding, need 1bit ~= 0.3us
static void encode_byte(uint8_t in, uint8_t* out) {
	uint32_t temp = 0;
	for(uint8_t i = 0; i < 8; i++) {
		temp <<= 4;
		if(in & 0x80) { temp |= 0b1110; }	//! 3*0.2667=0.8 high + 0.2667 low
		else { temp |= 0b1000; }			//! 0.2667 high + 3*0.2667=0.8 low
		in <<= 1;
	}
	*out = (temp >> 24) & 0xff;
	*(out + 1) = (temp >> 16) & 0xff;
	*(out + 2) = (temp >> 8) & 0xff;
	*(out + 3) = temp & 0xff;
}


extern void ws2812b_send(rgb_t *strip, uint16_t size) {
	uint8_t buffer[12 * size];

	for(uint16_t i = 0; i < size; i++) {
		encode_byte(strip[i].g, &buffer[12*i]);
		encode_byte(strip[i].r, &buffer[12*i+4]);
		encode_byte(strip[i].b, &buffer[12*i+8]);
	}

	spi_send_multiple_bytes_waiting(SPI3, buffer, 12*size);
	// LL_mDelay(1);
}



#elif WS2812B_DRIVE_METHOD == USE_DUMMY_IMPLEMENTATION



#define PIN_HIGH	LL_GPIO_SetOutputPin(DATA_PORT, DATA_PIN)
#define PIN_LOW		LL_GPIO_ResetOutputPin(DATA_PORT, DATA_PIN)
#define PIN_TOGGLE	LL_GPIO_TogglePin(DATA_PORT, DATA_PIN)

extern void ws2812b_init(void) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM10);

	LL_GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = DATA_PIN;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(DATA_PORT, &GPIO_InitStruct);
	PIN_LOW;

    //! Init Timer10 for precision delay
    LL_TIM_InitTypeDef LL_TIM_InitStruct;
    //! NOTE: 1tick = 0.1us
    LL_TIM_InitStruct.Prescaler = __LL_TIM_CALC_PSC(SystemCoreClock, 40000000);
    LL_TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
    LL_TIM_InitStruct.Autoreload = 0xffff; //! This will be set by the lib
    LL_TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    LL_TIM_InitStruct.RepetitionCounter = (uint8_t)0x00;
    LL_TIM_Init(TIM10, &LL_TIM_InitStruct);

    LL_TIM_EnableCounter(TIM10);
}

#define RESET_CPT	LL_TIM_SetCounter(TIM10, 0)
#define WAIT(x)		while(LL_TIM_GetCounter(TIM10) < (x))
#define LONG_TIME 	24
#define SHORT_TIME	10

static const uint8_t reverse_lookup[16] = {
0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf, };


uint8_t reverse(uint8_t n) {
   //! NOTE: Reverse the top and bottom nibble then swap them.
   return (reverse_lookup[n&0b1111] << 4) | reverse_lookup[n>>4];
}


extern void ws2812b_send(rgb_t *strip, uint16_t size) {
	uint8_t buffer[3 * size];

	for(uint16_t i = 0; i < size; i++) {
		buffer[i*3] 	= reverse(strip[i].g);
		buffer[i*3 + 1] = reverse(strip[i].r);
		buffer[i*3 + 2] = reverse(strip[i].b);
	}

	uint32_t i = 0, max = 3 * 8 * size;
	uint8_t high_time, low_time;

	__disable_irq();

	do {
		PIN_HIGH;
		RESET_CPT;
		if(buffer[i>>3] & 0x01) { high_time = LONG_TIME; low_time = SHORT_TIME; }
		else { high_time = SHORT_TIME - 2; low_time = LONG_TIME + 4; }
		WAIT(high_time);

		PIN_LOW;
		RESET_CPT;
		buffer[i>>3] >>= 2;
		i++;
		WAIT(low_time);
	} while(i < max);

	__enable_irq();

	RESET_CPT;
	WAIT(2000);
}

#endif
