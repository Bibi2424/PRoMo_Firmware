#define DEBUG_THIS_FILE DEBUG_SENSORS_FILE

#include <stdint.h>
#include <stdbool.h>

#include "global.h"
#include "debug.h"

#include "sensors.h"
#include "gpio.h"
#include "i2c.h"
#include "scheduler.h"


volatile bool is_busy = false;
const statInfo_t range_error = {
	.rawDistance = 65535,
	.signalCnt = 0,
	.ambientCnt = 0,
	.spadCnt = 0,
	.rangeStatus = 255,
};

static void vl53l0x_gpio_init(void) {
	LL_GPIO_InitTypeDef GPIO_InitStruct;

	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
	/**VL53L0X XSHUT Pins
	PC0   ------> VL53L0X_SHUT_1
	PC1   ------> VL53L0X_SHUT_2
	PC2   ------> VL53L0X_SHUT_3
	PC3   ------> VL53L0X_SHUT_4
	*/
	LL_GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.Pin = VL53L0X_XSHUT1_Pin | VL53L0X_XSHUT2_Pin | VL53L0X_XSHUT3_Pin | VL53L0X_XSHUT4_Pin;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(VL53L0X_XSHUT_Port, &GPIO_InitStruct);
	SET_PIN(VL53L0X_XSHUT_Port, VL53L0X_XSHUT1_Pin, 0);
	SET_PIN(VL53L0X_XSHUT_Port, VL53L0X_XSHUT2_Pin, 0);
	SET_PIN(VL53L0X_XSHUT_Port, VL53L0X_XSHUT3_Pin, 0);
	SET_PIN(VL53L0X_XSHUT_Port, VL53L0X_XSHUT4_Pin, 0);

	//! TODO: IRQ Pin

	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
	/**I2C1 GPIO Configuration
	PB8   ------> I2C1_SCL
	PB9   ------> I2C1_SDA
	*/
	LL_GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.Pin = VL53L0X_I2C_SCL | VL53L0X_I2C_SDA;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
	LL_GPIO_Init(VL53L0X_I2C_Port, &GPIO_InitStruct);
}


static uint32_t _get_sensor_pin(uint8_t sensor_id) {
	switch(sensor_id) {
	case VL53L0X_FRONT_ADDRESS:
		return VL53L0X_XSHUT2_Pin;
	case VL53L0X_LEFT_ADDRESS:
		return VL53L0X_XSHUT3_Pin;
	case VL53L0X_RIGHT_ADDRESS:
		return VL53L0X_XSHUT1_Pin;
	case VL53L0X_BACK_ADDRESS:
		return VL53L0X_XSHUT4_Pin;
	default:
		return 0;
	}
}


static bool sensor_init(uint8_t old_sensor_id, uint8_t new_sensor_id) {
	setAddress(old_sensor_id);
	uint32_t pin = _get_sensor_pin(new_sensor_id);
	if(pin == 0) { return false; }
	SET_PIN(VL53L0X_XSHUT_Port, pin, 1);
	if(initVL53L0X(true) == false) {
		return false;
		debugf("Error Init VL53L0X %u\n", new_sensor_id);
	}
	startContinuous(100);
	writeAddress(new_sensor_id);
	// LL_mDelay(1);

	setMeasurementTimingBudget( 50 * MILLIS );
	setTimeout( 50*2 );
	return true;
}


extern bool sensors_vl53l0x_init(void) {
	uint8_t fault = 0;

	vl53l0x_gpio_init();
	i2c1_init();

	fault += sensor_init(ADDRESS_DEFAULT, VL53L0X_FRONT_ADDRESS);
	fault += sensor_init(ADDRESS_DEFAULT, VL53L0X_LEFT_ADDRESS);
	fault += sensor_init(ADDRESS_DEFAULT, VL53L0X_RIGHT_ADDRESS);
	fault += sensor_init(ADDRESS_DEFAULT, VL53L0X_BACK_ADDRESS);

	return fault ? false : true;
}


//! For Single mode operation
extern uint16_t sensors_vl53l0x_range_one(uint8_t sensor_id, statInfo_t *range) {
	if(is_busy == true) { 
		memcpy(range, &range_error, sizeof(statInfo_t)); 
		return 65535; 
	}
	is_busy = true;
	setAddress(sensor_id);
	uint16_t error = readRangeSingleMillimeters( range );
	is_busy = false;
	if(timeoutOccurred() || error == 65535) { 
		memcpy(range, &range_error, sizeof(statInfo_t));
	}
	return error;
}


extern uint16_t sensors_vl53l0x_range_all(statInfo_t *ranges) {	
	uint16_t error = 0;
	for(uint8_t id = VL53L0X_FRONT_ADDRESS; id <= VL53L0X_BACK_ADDRESS; id += 2) {
		uint8_t i = (id - VL53L0X_FRONT_ADDRESS) / 2;
		if(sensors_vl53l0x_range_one(id, &ranges[i]) == 65535) { error = 65535; }
	}
	return error;
}


//! For continuous operation
extern uint16_t sensors_vl53l0x_get_one(uint8_t sensor_id, statInfo_t *range) {
	if(is_busy == true) { 
		memcpy(range, &range_error, sizeof(statInfo_t));
		return 65535; 
	}
	is_busy = true;
	setAddress(sensor_id);
	uint16_t error = readRangeContinuousMillimeters(range);
	is_busy = false;
	if(timeoutOccurred() || error == 65535) {
		memcpy(range, &range_error, sizeof(statInfo_t));
	}
	return error;
}


extern uint16_t sensors_vl53l0x_get_next(statInfo_t *range) {
	static uint8_t next_id = VL53L0X_FRONT_ADDRESS;

	uint16_t result = sensors_vl53l0x_get_one(next_id, range);

	next_id += 2;
	if(next_id > VL53L0X_BACK_ADDRESS) { next_id = VL53L0X_FRONT_ADDRESS; }

	return result;
}


extern uint16_t sensors_vl53l0x_get_all(statInfo_t *ranges) {
	uint16_t error = 0;
	for(uint8_t id = VL53L0X_FRONT_ADDRESS; id <= VL53L0X_BACK_ADDRESS; id += 2) {
		uint8_t i = (id - VL53L0X_FRONT_ADDRESS) / 2;
		if(sensors_vl53l0x_get_one(id, &ranges[i]) == 65535) { error = 65535; }
	}
	return error;
}
