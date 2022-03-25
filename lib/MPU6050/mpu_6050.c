#include "mpu_6050.h"

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_exti.h"

#include "i2c.h"


#define MPU_REG_SELF_TEST_X			0x0D
#define MPU_REG_SELF_TEST_Y			0x0E
#define MPU_REG_SELF_TEST_Z			0x0F
#define MPU_REG_SELF_TEST_A			0x10
#define MPU_REG_SMPLRT_DIV			0x19
#define MPU_REG_CONFIG				0x1A
#define MPU_REG_GYRO_CONFIG			0x1B
#define MPU_REG_ACCEL_CONFIG		0x1C
#define MPU_REG_FIFO_EN				0x23
#define MPU_REG_I2C_MST_CTRL		0x24
#define MPU_REG_I2C_SLV0_ADDR		0x25
#define MPU_REG_I2C_SLV0_REG		0x26
#define MPU_REG_I2C_SLV0_CTRL		0x27
#define MPU_REG_I2C_SLV1_ADDR		0x28
#define MPU_REG_I2C_SLV1_REG		0x29
#define MPU_REG_I2C_SLV1_CTRL 		0x2A
#define MPU_REG_I2C_SLV2_ADDR		0x2B
#define MPU_REG_I2C_SLV2_REG		0x2C
#define MPU_REG_I2C_SLV2_CTRL		0x2D
#define MPU_REG_I2C_SLV3_ADDR		0x2E
#define MPU_REG_I2C_SLV3_REG		0x2F
#define MPU_REG_I2C_SLV3_CTRL		0x30
#define MPU_REG_I2C_SLV4_ADDR		0x31
#define MPU_REG_I2C_SLV4_REG		0x32
#define MPU_REG_I2C_SLV4_DO			0x33
#define MPU_REG_I2C_SLV4_CTRL		0x34
#define MPU_REG_I2C_SLV4_DI			0x35
#define MPU_REG_I2C_MST_STATUS		0x36
#define MPU_REG_INT_PIN_CFG 		0x37
#define MPU_REG_INT_ENABLE			0x38
#define MPU_REG_INT_STATUS 			0x3A
#define MPU_REG_ACCEL_XOUT_H		0x3B
#define MPU_REG_ACCEL_XOUT_L		0x3C
#define MPU_REG_ACCEL_YOUT_H		0x3D
#define MPU_REG_ACCEL_YOUT_L		0x3E
#define MPU_REG_ACCEL_ZOUT_H		0x3F
#define MPU_REG_ACCEL_ZOUT_L		0x40
#define MPU_REG_TEMP_OUT_H			0x41
#define MPU_REG_TEMP_OUT_L			0x42
#define MPU_REG_GYRO_XOUT_H			0x43
#define MPU_REG_GYRO_XOUT_L			0x44
#define MPU_REG_GYRO_YOUT_H			0x45
#define MPU_REG_GYRO_YOUT_L			0x46
#define MPU_REG_GYRO_ZOUT_H			0x47
#define MPU_REG_GYRO_ZOUT_L			0x48
#define MPU_REG_EXT_SENS_DATA_00	0x49
#define MPU_REG_EXT_SENS_DATA_01	0x4A
#define MPU_REG_EXT_SENS_DATA_02	0x4B
#define MPU_REG_EXT_SENS_DATA_03	0x4C
#define MPU_REG_EXT_SENS_DATA_04	0x4D
#define MPU_REG_EXT_SENS_DATA_05	0x4E
#define MPU_REG_EXT_SENS_DATA_06	0x4F
#define MPU_REG_EXT_SENS_DATA_07	0x50
#define MPU_REG_EXT_SENS_DATA_08	0x51
#define MPU_REG_EXT_SENS_DATA_09	0x52
#define MPU_REG_EXT_SENS_DATA_10	0x53
#define MPU_REG_EXT_SENS_DATA_11	0x54
#define MPU_REG_EXT_SENS_DATA_12	0x55
#define MPU_REG_EXT_SENS_DATA_13	0x56
#define MPU_REG_EXT_SENS_DATA_14	0x57
#define MPU_REG_EXT_SENS_DATA_15	0x58
#define MPU_REG_EXT_SENS_DATA_16	0x59
#define MPU_REG_EXT_SENS_DATA_17	0x5A
#define MPU_REG_EXT_SENS_DATA_18	0x5B
#define MPU_REG_EXT_SENS_DATA_19	0x5C
#define MPU_REG_EXT_SENS_DATA_20	0x5D
#define MPU_REG_EXT_SENS_DATA_21	0x5E
#define MPU_REG_EXT_SENS_DATA_22	0x5F
#define MPU_REG_EXT_SENS_DATA_23	0x60
#define MPU_REG_I2C_SLV0_DO			0x63
#define MPU_REG_I2C_SLV1_DO			0x64
#define MPU_REG_I2C_SLV2_DO			0x65
#define MPU_REG_I2C_SLV3_DO			0x66
#define MPU_REG_I2C_MST_DELAY_CTRL	0x67
#define MPU_REG_SIGNAL_PATH_RESET	0x68
#define MPU_REG_USER_CTRL			0x6A
#define MPU_REG_PWR_MGMT_1			0x6B
#define MPU_REG_PWR_MGMT_2			0x6C
#define MPU_REG_FIFO_COUNTH			0x72
#define MPU_REG_FIFO_COUNTL 		0x73
#define MPU_REG_FIFO_R_W 			0x74
#define MPU_REG_WHO_AM_I			0x75


static uint8_t mpu_address = 0;


static void mpu_6050_gpio_init(void) {
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);

	LL_GPIO_InitTypeDef GPIO_InitStruct;
	LL_EXTI_InitTypeDef EXTI_InitStruct;

	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
	//! PC13  ------> FSYNC
	LL_GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.Pin = LL_GPIO_PIN_13;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	// SET_PIN(GPIOC, LL_GPIO_PIN_13, 0);

	//! PC14  ------> INT
	LL_GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.Pin = LL_GPIO_PIN_14;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
	LL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	LL_SYSCFG_SetEXTISource(LL_SYSCFG_EXTI_PORTC, LL_SYSCFG_EXTI_LINE14);
	EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_14;
	EXTI_InitStruct.LineCommand = ENABLE;
	EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;
	EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;
	LL_EXTI_Init(&EXTI_InitStruct);

	NVIC_SetPriority(EXTI15_10_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 4, 0));
	NVIC_EnableIRQ(EXTI15_10_IRQn);

	/**I2C3 GPIO Configuration
	PA8   ------> I2C3_SCL
	PC9   ------> I2C3_SDA
	*/
	LL_GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.Pin = LL_GPIO_PIN_8;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
	GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
	LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
	LL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}


//* IRQ Line *******************************************************************
void EXTI15_10_IRQHandler(void) {
	if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_14) != RESET) {
		LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_14);
		// printf("MPU INT\n");
	}
}


extern bool mpu_6050_init(uint8_t address) {
	mpu_6050_gpio_init();
	i2c3_init();

	//! 7bits addressing
	mpu_address = address << 1;

	uint8_t value;
    uint8_t err;

    //! Verify communication
    err = i2c3_full_read(mpu_address, MPU_REG_WHO_AM_I, &value, 1);
    if(err != 0 || value != 0x68) {
    	return false;
    }

    //! Wakeup device
    value = 0;
    err |= i2c3_full_write(mpu_address, MPU_REG_PWR_MGMT_1, &value, 1);

    //! Filter gyroscope/accel 2 -> ~3ms delay, 
    value = 2;
    err |= i2c3_full_write(mpu_address, MPU_REG_CONFIG, &value, 1);

    // //! Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
    // value = 0x07;
    // err |= i2c3_full_write(mpu_address, MPU_REG_SMPLRT_DIV, &value, 1);

    //! Gyro Full scale range of ± 250 °/s
    //! Accel Full scale range of ± 2g	(consecutive registers)
    uint8_t data[2] = {0, 0};
    err |= i2c3_full_write(mpu_address, MPU_REG_GYRO_CONFIG, data, 2);

    //! Enable Interrupt (bit0: data, bit3: i2c_slave, bit4: fifo ovflw)
    value = 1;
    err |= i2c3_full_write(mpu_address, MPU_REG_INT_ENABLE, &value, 1);

    return err == 0 ? true: false;
}


extern bool mpu_6050_reset_instruments(bool gyro, bool accel, bool temp) {
	uint8_t value = 0;
	if(gyro) { value |= 1 << 2; }
	if(accel) { value |= 1 << 1; }
	if(temp) { value |= 1; }

    uint8_t err = i2c3_full_write(mpu_address, MPU_REG_SIGNAL_PATH_RESET, &value, 1);

    return err == 0 ? true: false;
}



extern bool mpu_6050_read_accel(mpu_accel_t *accel_data) {
	uint8_t accel_raw[6];
	uint8_t err =  i2c3_full_read(mpu_address, MPU_REG_ACCEL_XOUT_H, accel_raw, 6);

	accel_data->x = ((int16_t)accel_raw[0]) << 8 | accel_raw[1];
	accel_data->y = ((int16_t)accel_raw[2]) << 8 | accel_raw[3];
	accel_data->z = ((int16_t)accel_raw[4]) << 8 | accel_raw[5];

	/** NOTE: Sensitivity
	0	±2g		16384 LSB/g
	1	±4g		8192 LSB/g
	2	±8g		4096 LSB/g
	3	±16g	2048 LSB/g
	**/

    return err == 0 ? true: false;
}


extern bool mpu_6050_read_gyro(mpu_gyro_t *gyro_data) {
	uint8_t gyro_raw[6];
	uint8_t err =  i2c3_full_read(mpu_address, MPU_REG_ACCEL_XOUT_H, gyro_raw, 6);

	gyro_data->x = ((int16_t)gyro_raw[0]) << 8 | gyro_raw[1];
	gyro_data->y = ((int16_t)gyro_raw[2]) << 8 | gyro_raw[3];
	gyro_data->z = ((int16_t)gyro_raw[4]) << 8 | gyro_raw[5];

	/** NOTE: Sensitivity
	0	± 250 °/s	131 LSB/°/s
	1	± 500 °/s	65.5 LSB/°/s
	2	± 1000 °/s	32.8 LSB/°/s
	3	± 2000 °/s	16.4 LSB/°/s
	**/

    return err == 0 ? true: false;
}


extern bool mpu_6050_read_temp(uint16_t *temp_data) {
	uint8_t temp_raw[2];
	uint8_t err =  i2c3_full_read(mpu_address, MPU_REG_TEMP_OUT_H, temp_raw, 2);

	*temp_data = ((int16_t)temp_raw[0]) << 8 | temp_raw[1];

	//! Temperature in degrees C = (TEMP_OUT Register Value as a signed quantity)/340 + 36.53

    return err == 0 ? true: false;
}


extern bool mpu_6050_read_all(mpu_data_t *data) {
	uint8_t data_raw[14];
	uint8_t err =  i2c3_full_read(mpu_address, MPU_REG_ACCEL_XOUT_H, data_raw, 14);

	data->accel.x = ((int16_t)data_raw[0]) << 8 | data_raw[1];
	data->accel.y = ((int16_t)data_raw[2]) << 8 | data_raw[3];
	data->accel.z = ((int16_t)data_raw[4]) << 8 | data_raw[5];

	data->temp = ((int16_t)data_raw[6]) << 8 | data_raw[7];

	data->gyro.x = ((int16_t)data_raw[8]) << 8 | data_raw[9];
	data->gyro.y = ((int16_t)data_raw[10]) << 8 | data_raw[11];
	data->gyro.z = ((int16_t)data_raw[12]) << 8 | data_raw[13];

    return err == 0 ? true: false;
}
