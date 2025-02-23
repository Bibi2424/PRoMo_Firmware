add_library(logger
	logger.c
)

target_include_directories(logger PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)
