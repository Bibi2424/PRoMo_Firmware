add_library(stm32_bsp
	gpio.c
	i2c.c
	spi.c
	ws2812b.c
)

target_link_libraries( stm32_bsp PUBLIC stm32hal )

target_include_directories(stm32_bsp PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)