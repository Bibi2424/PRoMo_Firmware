#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "promo_v0_1_bsp.h"

#define spi2_init() 							spi_init(SPI2)
#define spi2_send_byte_waiting(write_data, read_data) \
												spi_send_byte_waiting(SPI2, write_data, read_data)
#define spi2_send_multiple_bytes_waiting(write_data, read_data, size) \
												spi_send_multiple_bytes_waiting(SPI2, write_data, read_data, size)


extern void spi_init(SPI_TypeDef *SPIx);
extern bool spi_send_byte_waiting(SPI_TypeDef *SPIx, uint8_t write_data, uint8_t *read_data);
extern bool spi_send_multiple_bytes_waiting(SPI_TypeDef *SPIx, uint8_t* write_data, uint8_t* read_data, uint8_t size);

#endif
