add_executable( ${PROJECT_NAME} 
        main.c
        drive_speed_control.c
        encoder.c
        motor.c
        nrf24l01.c
        process_serial_commands.c
        radio.c
        scheduler.c
        sensors.c
        usart.c
)

# pull in common dependencies
target_link_libraries( ${PROJECT_NAME} PUBLIC
        bsp
        mpu6050
        promo_lib
        vl53l0x
        # stm32hal
)

target_include_directories( ${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

# configure_file(project_description.h.in ${CMAKE_CURRENT_LIST_DIR}/project_description.h)

set(FW_VERSION_MAJOR "0")
set(FW_VERSION_MINOR "2")
set(FW_VERSION_REV   "3")

set(HW_TYPE "HW_PROMO_V0_1")

target_compile_definitions(${PROJECT_NAME} PUBLIC 
    TARGET=${PROJECT_NAME}

    FW_VERSION_MAJOR=${FW_VERSION_MAJOR}
    FW_VERSION_MINOR=${FW_VERSION_MINOR}
    FW_VERSION_REV=${FW_VERSION_REV}
    HW_TYPE=${HW_TYPE}
)
if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(${PROJECT_NAME} PUBLIC 
        DEBUG_ENABLE=1
    )
endif()

# Create binary file
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY} -O binary ${PROJECT_NAME}.elf ${PROJECT_NAME}.bin)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY} -O ihex ${PROJECT_NAME}.elf ${PROJECT_NAME}.hex)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJDUMP} -d -S ${PROJECT_NAME}.elf > ${PROJECT_NAME}.dmp)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_SIZE} ${CMAKE_PROJECT_NAME}.elf)
