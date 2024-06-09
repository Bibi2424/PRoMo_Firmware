#ifndef __I2C_H__
#define __I2C_H__

#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"

#define SCL_CLOCK  100000L
// #define SCL_CLOCK  400000L

#define I2C_READ    1
#define I2C_WRITE   0

#define I2C_NO_ERROR	0

#ifndef TM_I2C_TIMEOUT
#define TM_I2C_TIMEOUT	10
#endif


#define i2c1_init() 									i2c_init(I2C1)
#define i2c1_full_write(address, mem, buffer, size)		i2c_full_write(I2C1, address, mem, buffer, size)
#define i2c1_full_read(address, mem, buffer, size)		i2c_full_read(I2C1, address, mem, buffer, size)

// #define searchI2C1()									searchI2C(I2C1)


#define i2c2_init() 									i2c_init(I2C2)
#define i2c2_full_write(address, mem, buffer, size)		i2c_full_write(I2C2, address, mem, buffer, size)
#define i2c2_full_read(address, mem, buffer, size)		i2c_full_read(I2C2, address, mem, buffer, size)

// #define searchI2C2()									searchI2C(I2C2)


#define i2c3_init() 									i2c_init(I2C3)
#define i2c3_full_write(address, mem, buffer, size)		i2c_full_write(I2C3, address, mem, buffer, size)
#define i2c3_full_read(address, mem, buffer, size)		i2c_full_read(I2C3, address, mem, buffer, size)

// #define searchI2C3()									searchI2C(I2C3)


extern void i2c_init(I2C_TypeDef *I2Cx);
extern uint8_t i2c_full_write(I2C_TypeDef *I2Cx, uint8_t address, uint8_t mem, uint8_t const *buffer, uint8_t size);
extern uint8_t i2c_full_read(I2C_TypeDef *I2Cx, uint8_t address, uint8_t mem, uint8_t *buffer, uint8_t size);

// extern void searchI2C(I2C_TypeDef *I2Cx);


#endif
