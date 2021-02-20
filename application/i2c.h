#ifndef __I2C_H__
#define __I2C_H__

#include "main.h"

#define SCL_CLOCK  100000L
// #define SCL_CLOCK  400000L

#define I2C_READ    1
#define I2C_WRITE   0

#ifndef TM_I2C_TIMEOUT
#define TM_I2C_TIMEOUT					5
#endif


#define i2c1_init() 									i2c_init(I2C1)
#define i2c1_full_write(address, mem, buffer, size)		i2c_full_write(I2C1, address, mem, buffer, size)
#define i2c1_full_read(address, mem, buffer, size)		i2c_full_read(I2C1, address, mem, buffer, size)
// #define i2c1_start(addr) 								i2c_start(I2C1, addr)
// #define i2c1_rep_start(addr)							i2c_rep_start(I2C1, addr)
// #define i2c1_write(data) 								i2c_write(I2C1, data)
// #define i2c1_readAck(out)								i2c_readAck(I2C1, out)
// #define i2c1_readNak(out) 								i2c_readNak(I2C1, out)
// #define i2c1_read(ack, out)  							i2c_read(I2C1, ack, out)
// #define i2c1_stop()										i2c_stop(I2C1)
// #define searchI2C1()									searchI2C(I2C1)



extern void i2c_init(I2C_TypeDef *I2Cx);
extern uint8_t i2c_full_write(I2C_TypeDef *I2Cx, uint8_t address, uint8_t mem, uint8_t const *buffer, uint8_t size);
extern uint8_t i2c_full_read(I2C_TypeDef *I2Cx, uint8_t address, uint8_t mem, uint8_t *buffer, uint8_t size);
// extern uint8_t i2c_start(I2C_TypeDef *I2Cx, uint8_t addr);
// extern uint8_t i2c_rep_start(I2C_TypeDef *I2Cx, uint8_t addr);
// extern uint8_t i2c_write(I2C_TypeDef *I2Cx, uint8_t data);
// extern uint8_t i2c_readAck(I2C_TypeDef *I2Cx, uint8_t *out);
// extern uint8_t i2c_readNak(I2C_TypeDef *I2Cx, uint8_t *out);
// #define i2c_read(I2Cx, ack, out)  (ack) ? i2c_readAck(I2Cx, out) : i2c_readNak(I2Cx, out);
// extern uint8_t i2c_stop(I2C_TypeDef *I2Cx);
// extern void searchI2C(I2C_TypeDef *I2Cx);


#endif
