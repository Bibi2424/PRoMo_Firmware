cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

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

# Include toolchain file
set(STM32_HAL_FOLDER STM32_HAL_MIN)
set(STARTUP_FOLDER bsp/PRoMo_v0_1)
include("gcc-arm-none-eabi.cmake")
include("stm32_hal.cmake")

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

add_subdirectory(bsp/PRoMo_v0_1)
add_subdirectory(lib/MPU6050)
add_subdirectory(lib/PRoMo_lib)
add_subdirectory(lib/VL53L0X)

add_subdirectory(application)
