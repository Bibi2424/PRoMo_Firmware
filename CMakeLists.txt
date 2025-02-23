cmake_minimum_required(VERSION 3.22)

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME PRoMo_fw)

# MCU Configuration
set(MCU STM32F405)
# set(MCU STM32F411)
if(MCU MATCHES STM32F405)
    set(STARTUP_FILE startup_stm32f405xx.s)
    set(LINKER_FILE STM32F405RGTx_FLASH.ld)
    set(MCU_TYPE STM32F405xx)
    message(STATUS "STM32F405 Selected")
elseif(MCU MATCHES STM32F411)
    set(STARTUP_FILE startup_stm32f411xe.s)
    set(LINKER_FILE STM32F411RETx_FLASH.ld)
    set(MCU_TYPE STM32F411xE)
    message(STATUS "STM32F411 Selected")
else()
    message( FATAL_ERROR "You need to select a supported MCU" )
endif()

# Include toolchain file
set(STM32_HAL_FOLDER STM32CubeF4/Drivers)
set(STARTUP_FOLDER bsp/PRoMo_v0_1)
include("cmake/gcc-arm-none-eabi.cmake")
include("cmake/stm32_hal.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

add_subdirectory(bsp/PRoMo_v0_1)
add_subdirectory(lib/MPU6050)
add_subdirectory(lib/PRoMo_lib)
add_subdirectory(lib/VL53L0X)
add_subdirectory(lib/NRF24)

add_subdirectory(application)
