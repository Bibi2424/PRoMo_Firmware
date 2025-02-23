#ifndef __SPI_H__
#define __SPI_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "bsp.h"


extern void spi_init(SPI_TypeDef *SPIx);
extern bool spi_send_byte_waiting(SPI_TypeDef *SPIx, uint8_t write_data, uint8_t *read_data);
extern bool spi_send_multiple_bytes_waiting(SPI_TypeDef *SPIx, uint8_t* write_data, uint8_t* read_data, uint8_t size);

static inline void spi2_init(void) { 
    spi_init(SPI2); 
}
static inline bool spi2_send_byte_waiting(uint8_t write_data, uint8_t *read_data) { 
    return spi_send_byte_waiting(SPI2, write_data, read_data); 
}
static inline bool spi2_send_multiple_bytes_waiting(uint8_t *write_data, uint8_t *read_data, uint8_t size) { 
    return spi_send_multiple_bytes_waiting(SPI2, write_data, read_data, size); 
}

#endif
