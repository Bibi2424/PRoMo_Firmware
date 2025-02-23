#ifndef __I2C_H__
#define __I2C_H__

#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"

#define I2C_READ    1
#define I2C_WRITE   0

#define I2C_NO_ERROR	0

#ifndef TM_I2C_TIMEOUT
#define TM_I2C_TIMEOUT	10
#endif

extern void i2c_init(I2C_TypeDef *I2Cx, uint32_t clock_speed);
extern uint8_t i2c_full_write(I2C_TypeDef *I2Cx, uint8_t address, uint8_t mem, uint8_t const *buffer, uint8_t size);
extern uint8_t i2c_full_read(I2C_TypeDef *I2Cx, uint8_t address, uint8_t mem, uint8_t *buffer, uint8_t size);

static inline void i2c1_init(uint32_t clock_speed) { i2c_init(I2C1, clock_speed); }
static inline uint8_t i2c1_full_write(uint8_t address, uint8_t mem, uint8_t const *buffer, uint8_t size) { return i2c_full_write(I2C1, address, mem, buffer, size); }
static inline uint8_t i2c1_full_read(uint8_t address, uint8_t mem, uint8_t *buffer, uint8_t size) { return i2c_full_read(I2C1, address, mem, buffer, size); }

static inline void i2c2_init(uint32_t clock_speed) { i2c_init(I2C2, clock_speed); }
static inline uint8_t i2c2_full_write(uint8_t address, uint8_t mem, uint8_t const *buffer, uint8_t size) { return i2c_full_write(I2C2, address, mem, buffer, size); }
static inline uint8_t i2c2_full_read(uint8_t address, uint8_t mem, uint8_t *buffer, uint8_t size) { return i2c_full_read(I2C2, address, mem, buffer, size); }

static inline void i2c3_init(uint32_t clock_speed) { i2c_init(I2C3, clock_speed); }
static inline uint8_t i2c3_full_write(uint8_t address, uint8_t mem, uint8_t const *buffer, uint8_t size) { return i2c_full_write(I2C3, address, mem, buffer, size); }
static inline uint8_t i2c3_full_read(uint8_t address, uint8_t mem, uint8_t *buffer, uint8_t size) { return i2c_full_read(I2C3, address, mem, buffer, size); }

#endif
