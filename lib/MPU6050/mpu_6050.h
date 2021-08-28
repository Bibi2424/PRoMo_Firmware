#ifndef MPU_6050_H
#define MPU_6050_H
#include <stdint.h>
#include <stdbool.h>


#define MPU_6050_DEFAULT_ADDRESS	0x68


typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} mpu_accel_t;

typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
} mpu_gyro_t;

typedef struct {
	mpu_accel_t accel;
	mpu_gyro_t gyro;
	int16_t temp;
} mpu_data_t;


extern bool mpu_6050_init(uint8_t address);
extern bool mpu_6050_reset_instruments(bool gyro, bool accel, bool temp);

extern bool mpu_6050_read_accel(mpu_accel_t *accel_data);
extern bool mpu_6050_read_gyro(mpu_gyro_t *gyro_data);
extern bool mpu_6050_read_temp(uint16_t *temp_data);
extern bool mpu_6050_read_all(mpu_data_t *data);


#endif