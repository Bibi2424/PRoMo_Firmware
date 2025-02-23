add_library(NRF24
	nrf24l01.c
)

target_link_libraries( NRF24 PUBLIC bsp )

target_include_directories(NRF24 PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)