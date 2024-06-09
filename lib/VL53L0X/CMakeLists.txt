add_library(vl53l0x
	VL53L0X.c
)

target_link_libraries( vl53l0x PUBLIC bsp )

target_include_directories(vl53l0x PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)