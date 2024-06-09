add_executable( ${CMAKE_PROJECT_NAME} 
        main.c
        drive_speed_control.c
        encoder.c
        motor.c
        nrf24l01.c
        process_serial_commands.c
        radio.c
        scheduler.c
        sensors.c
        syscalls.c
        usart.c
)

# pull in common dependencies
target_link_libraries( ${CMAKE_PROJECT_NAME} PUBLIC
        bsp
        mpu6050
        promo_lib
        vl53l0x
        # stm32hal
)

target_include_directories( ${CMAKE_PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)


# Create binary file
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin)
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.hex)
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJDUMP} -d -S ${CMAKE_PROJECT_NAME}.elf > ${CMAKE_PROJECT_NAME}.dmp)
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_SIZE} ${CMAKE_PROJECT_NAME}.elf)
