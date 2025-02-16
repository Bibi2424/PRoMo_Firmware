add_library(promo_lib
	odometry.c
	control_loop.c
	pid_controller.c
)

target_include_directories(promo_lib PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)