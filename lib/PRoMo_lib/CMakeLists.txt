add_library(promo_lib
	control_loop.c
	pid_controller.c
)

target_include_directories(promo_lib PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)