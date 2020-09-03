cmake_minimum_required (VERSION 2.8)

project (nrf_app)

include_directories (${PROJECT_SOURCE_DIR}/include)

set (SRC_TX
	${PROJECT_SOURCE_DIR}/src/nrf24l01.c
	${PROJECT_SOURCE_DIR}/src/list.c
	${PROJECT_SOURCE_DIR}/src/nrf_tx.c
	)

set (SRC_RX
	${PROJECT_SOURCE_DIR}/src/nrf24l01.c
	${PROJECT_SOURCE_DIR}/src/list.c
	${PROJECT_SOURCE_DIR}/src/nrf_rx.c
	)

add_executable(nrf_tx ${SRC_TX})
add_executable(nrf_rx ${SRC_RX})

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

SET(CMAKE_C_COMPILER "/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-gcc")
SET(CMAKE_CXX_COMPILER "/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-g++")




