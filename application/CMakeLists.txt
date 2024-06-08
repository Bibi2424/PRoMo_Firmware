add_executable( ${CMAKE_PROJECT_NAME} 
        main.c
        drive_speed_control.c
        encoder.c
        lerp.c
        motor.c
        nrf24l01.c
        process_serial_commands.c
        radio.c
        scheduler.c
        sensors.c
        syscalls.c
        system_stm32f4xx.c
        usart.c
)

# pull in common dependencies
target_link_libraries( ${CMAKE_PROJECT_NAME} PUBLIC
        stm32_bsp
        mpu6050
        promo_lib
        vl53l0x
        stm32hal
)

target_include_directories( ${CMAKE_PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
