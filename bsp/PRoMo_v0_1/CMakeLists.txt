add_library(bsp
	gpio.c
	i2c.c
	spi.c
	ws2812b.c
	time.c
	stm32f4xx_it.c
    system_stm32f4xx.c
	syscalls.c
	sysmem.c
)

target_link_libraries( bsp PUBLIC stm32hal )

target_include_directories(bsp PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)