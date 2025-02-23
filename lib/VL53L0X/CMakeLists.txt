add_library(vl53l0x
	VL53L0X.c
)

target_include_directories(vl53l0x PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
