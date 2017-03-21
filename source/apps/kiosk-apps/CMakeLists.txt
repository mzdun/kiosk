cmake_minimum_required(VERSION 3.4)
project(kiosk-apps C CXX)

add_custom_target(${PROJECT_NAME}
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/default" "${CMAKE_CURRENT_BINARY_DIR}/../apps/default"
	VERBATIM
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/default DESTINATION ${SHARE_ROOT}apps/)