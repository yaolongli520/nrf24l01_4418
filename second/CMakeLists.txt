cmake_minimum_required (VERSION 3.5)

project (nrf_app)

include_directories (${PROJECT_SOURCE_DIR}/include)

set (SRC_CLIENT
	${PROJECT_SOURCE_DIR}/src/nrf24l01.cpp
	${PROJECT_SOURCE_DIR}/src/list.cpp
	${PROJECT_SOURCE_DIR}/src/mytime.cpp
	${PROJECT_SOURCE_DIR}/src/pack.cpp
	${PROJECT_SOURCE_DIR}/src/nrf_client.cpp)

set (SRC_SERVER
	${PROJECT_SOURCE_DIR}/src/nrf24l01.cpp
	${PROJECT_SOURCE_DIR}/src/list.cpp
	${PROJECT_SOURCE_DIR}/src/mytime.cpp
	${PROJECT_SOURCE_DIR}/src/pack.cpp
	${PROJECT_SOURCE_DIR}/src/nrf_server.cpp)

add_executable(nrf_client ${SRC_CLIENT})
add_executable(nrf_server ${SRC_SERVER})

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

SET(CMAKE_C_COMPILER "/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-gcc")
SET(CMAKE_CXX_COMPILER "/opt/FriendlyARM/toolchain/4.9.3/bin/arm-linux-g++")


