#ifndef _SENSORS_H
#define _SENSORS_H

#include "main.h"
#include "VL53L0X.h"

#define VL53L0X_FRONT_ADDRESS 	0x60
#define VL53L0X_LEFT_ADDRESS 	0x62
#define VL53L0X_RIGHT_ADDRESS 	0x64
#define VL53L0X_BACK_ADDRESS 	0x66

typedef enum {
	// Defined by the specs
	VL53L1_RANGESTATUS_RANGE_VALID = 0,
	VL53L1_RANGESTATUS_SIGMA_FAIL = 1,
	VL53L1_RANGESTATUS_SIGNAL_FAIL = 2,
	VL53L1_RANGESTATUS_RANGE_VALID_MIN_RANGE_CLIPPED = 3,
	VL53L1_RANGESTATUS_OUTOFBOUNDS_FAIL = 4,
	VL53L1_RANGESTATUS_HARDWARE_FAIL = 5,
	VL53L1_RANGESTATUS_RANGE_VALID_NO_WRAP_CHECK_FAIL = 6,
	VL53L1_RANGESTATUS_WRAP_TARGET_FAIL = 7,
	VL53L1_RANGESTATUS_PROCESSING_FAIL = 8,
	VL53L1_RANGESTATUS_XTALK_SIGNAL_FAIL = 9,
	VL53L1_RANGESTATUS_SYNCRONISATION_INT = 10,
	VL53L1_RANGESTATUS_RANGE_VALID_MERGED_PULSE = 11,
	VL53L1_RANGESTATUS_TARGET_PRESENT_LACK_OF_SIGNAL = 12,
	VL53L1_RANGESTATUS_MIN_RANGE_FAIL = 13,
	VL53L1_RANGESTATUS_RANGE_INVALID = 14,
	VL53L1_RANGESTATUS_NONE = 255,
	// Custom ones
	VL53L1_RANGESTATUS_TIMEOUT = 251,
	VL53L1_RANGESTATUS_RANGE_INFINY = 252,
	VL53L1_RANGESTATUS_RESSOURCE_BUSY = 253,
	VL53L1_RANGESTATUS_GENERIC_ERROR = 254,
} range_status_t;


extern bool sensors_vl53l0x_init(void);

static inline bool sensors_vl53l0x_is_status_ok(range_status_t status) {
	if(status == VL53L1_RANGESTATUS_RANGE_VALID || status == VL53L1_RANGESTATUS_RANGE_VALID_NO_WRAP_CHECK_FAIL || status == VL53L1_RANGESTATUS_RANGE_VALID_MERGED_PULSE) { return true; }
	return false;
}

extern range_status_t sensors_vl53l0x_range_one(uint8_t sensor_id, statInfo_t *range);
extern range_status_t sensors_vl53l0x_range_all(statInfo_t *ranges);

//! For continuous operation
extern range_status_t sensors_vl53l0x_get_one(uint8_t sensor_id, statInfo_t *range);
extern range_status_t sensors_vl53l0x_get_next(statInfo_t *range);
extern range_status_t sensors_vl53l0x_get_all(statInfo_t *ranges);

#endif