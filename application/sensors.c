#define DEBUG_THIS_FILE	DEBUG_SENSORS_FILE

#include "sensors.h"
#include "global.h"
#include "gpio.h"
#include "i2c.h"


static void VL53L0X_gpio_init(void) {
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
    /**VL53L0X XSHUT Pins
    PC0   ------> VL53L0X_SHUT_1
    PC1   ------> VL53L0X_SHUT_2
    PC2   ------> VL53L0X_SHUT_3
    PC3   ------> VL53L0X_SHUT_4
    */
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

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    /**I2C1 GPIO Configuration
    PB8   ------> I2C1_SCL
    PB9   ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = VL53L0X_I2C_SCL | VL53L0X_I2C_SDA;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
    LL_GPIO_Init(VL53L0X_I2C_Port, &GPIO_InitStruct);
}


extern void sensors_VL53L0X_init(void) {
	bool res;
	statInfo_t xTraStats;

	VL53L0X_gpio_init();
	i2c1_init();

	setAddress(ADDRESS_DEFAULT);
    SET_PIN(VL53L0X_XSHUT_Port, VL53L0X_FRONT_PIN, 1);
	res = initVL53L0X(1);
	if(res == false) { printf("Error Init VL53L0X FRONT\n"); }
	writeAddress(VL53L0X_FRONT_ADDRESS);
	// LL_mDelay(1);

	setAddress(ADDRESS_DEFAULT);
    SET_PIN(VL53L0X_XSHUT_Port, VL53L0X_LEFT_PIN, 1);
	res = initVL53L0X(1);
	if(res == false) { printf("Error Init VL53L0X LEFT\n"); }
	writeAddress(VL53L0X_LEFT_ADDRESS);
	// LL_mDelay(1);

	setAddress(ADDRESS_DEFAULT);
    SET_PIN(VL53L0X_XSHUT_Port, VL53L0X_RIGHT_PIN, 1);
	res = initVL53L0X(1);
	if(res == false) { printf("Error Init VL53L0X RIGHT\n"); }
	writeAddress(VL53L0X_RIGHT_ADDRESS);
	// LL_mDelay(1);

	setAddress(ADDRESS_DEFAULT);
    SET_PIN(VL53L0X_XSHUT_Port, VL53L0X_BACK_PIN, 1);
	res = initVL53L0X(1);
	if(res == false) { printf("Error Init VL53L0X BACK\n"); }
	writeAddress(VL53L0X_BACK_ADDRESS);

	setMeasurementTimingBudget( 50 * MILLIS );
}


extern uint16_t sensors_VL53L0X_range_one(uint8_t sensor_id, statInfo_t *range) {
	setAddress(sensor_id);
	return readRangeSingleMillimeters( range );
}


extern uint16_t sensors_VL53L0X_range_all(statInfo_t *ranges) {
	uint16_t error = 0;;
	setAddress(VL53L0X_FRONT_ADDRESS);
	if(readRangeSingleMillimeters( &ranges[0] ) == 65535) { error = 65535; }
	setAddress(VL53L0X_LEFT_ADDRESS);
	if(readRangeSingleMillimeters( &ranges[1] ) == 65535) { error = 65535; }
	setAddress(VL53L0X_RIGHT_ADDRESS);
	if(readRangeSingleMillimeters( &ranges[2] ) == 65535) { error = 65535; }
	setAddress(VL53L0X_BACK_ADDRESS);
	if(readRangeSingleMillimeters( &ranges[3] ) == 65535) { error = 65535; }
	return error;
}
