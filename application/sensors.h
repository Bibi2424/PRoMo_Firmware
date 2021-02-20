#ifndef _SENSORS_H
#define _SENSORS_H

#include "main.h"
#include "VL53L0X.h"

#define VL53L0X_FRONT_ADDRESS 	0x60
#define VL53L0X_LEFT_ADDRESS 	0x62
#define VL53L0X_RIGHT_ADDRESS 	0x64
#define VL53L0X_BACK_ADDRESS 	0x66

#define VL53L0X_FRONT_PIN 	VL53L0X_XSHUT2_Pin
#define VL53L0X_LEFT_PIN 	VL53L0X_XSHUT3_Pin
#define VL53L0X_RIGHT_PIN 	VL53L0X_XSHUT1_Pin
#define VL53L0X_BACK_PIN 	VL53L0X_XSHUT4_Pin


extern void sensors_VL53L0X_init(void);
extern uint16_t sensors_VL53L0X_range_one(uint8_t sensor_id, statInfo_t *range);
extern uint16_t sensors_VL53L0X_range_all(statInfo_t *ranges);

#endif