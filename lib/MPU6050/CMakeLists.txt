add_library(mpu6050 
	mpu_6050.c
)

target_link_libraries( mpu6050 PUBLIC bsp )

target_include_directories(mpu6050 PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)