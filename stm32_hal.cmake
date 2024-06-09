add_library(stm32hal 
    ${STM32_HAL_FOLDER}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_gpio.c
    ${STM32_HAL_FOLDER}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_dma.c
    ${STM32_HAL_FOLDER}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_i2c.c
    ${STM32_HAL_FOLDER}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rcc.c
    ${STM32_HAL_FOLDER}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rtc.c
    ${STM32_HAL_FOLDER}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_pwr.c
    ${STM32_HAL_FOLDER}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_utils.c
    ${STM32_HAL_FOLDER}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_exti.c
    ${STM32_HAL_FOLDER}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rng.c
    ${STM32_HAL_FOLDER}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_spi.c
    ${STM32_HAL_FOLDER}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_tim.c
    ${STM32_HAL_FOLDER}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usart.c
    ${STARTUP_FOLDER}/startup_stm32f405xx.s
)

target_compile_definitions(stm32hal PUBLIC 
	USE_FULL_LL_DRIVER 
	HSE_VALUE=25000000 
	HSE_STARTUP_TIMEOUT=100 
	LSE_STARTUP_TIMEOUT=5000 
	LSE_VALUE=32768 
	EXTERNAL_CLOCK_VALUE=12288000 
	HSI_VALUE=16000000 
	LSI_VALUE=32000 
	VDD_VALUE=3300 
	PREFETCH_ENABLE=1 
	INSTRUCTION_CACHE_ENABLE=1 
	DATA_CACHE_ENABLE=1 
	STM32F405xx
    $<$<CONFIG:Debug>:DEBUG>
)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(stm32hal PUBLIC 
        DEBUG_ENABLE=1
    )
endif()

# include_directories(
target_include_directories(stm32hal PUBLIC
    ${STM32_HAL_FOLDER}/STM32F4xx_HAL_Driver/Inc
    ${STM32_HAL_FOLDER}/CMSIS/Device/ST/STM32F4xx/Include
    ${STM32_HAL_FOLDER}/CMSIS/Include
)

target_link_directories(stm32hal PUBLIC
)

target_link_libraries(stm32hal PUBLIC
)

# Validate that STM32CubeMX code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()


