cmake_minimum_required(VERSION 3.4)
project(panel CXX)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/version.h.in ${PROJECT_BINARY_DIR}/version.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SRC
	main.cc
)

add_executable(${PROJECT_NAME} ${SRC})
install (TARGETS ${PROJECT_NAME} DESTINATION bin)