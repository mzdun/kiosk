cmake_minimum_required(VERSION 3.4)
project(kiosk C CXX)

if (UNIX)
set(LAUNCHER_DIRSEP "/")
set(LAUNCHER_PATHSEP ":")
else()
set(LAUNCHER_DIRSEP "\\\\")
set(LAUNCHER_PATHSEP ";")
endif()

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/version.h.in ${PROJECT_BINARY_DIR}/version.h)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/paths.h.in ${PROJECT_BINARY_DIR}/paths.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../kiosk-app)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if (UNIX)
set(POSTFIX "linux")
elseif (WIN32)
set(POSTFIX "win")
endif()

set(SRCS
	# main.cc
	main_${POSTFIX}.cc
	)

if (UNIX)
add_executable(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})
message(STATUS "dl: ${CMAKE_DL_LIBS}")
elseif (WIN32)
list(APPEND SRCS
	kiosk.rc
	)

add_definitions(
-DUNICODE
-D_UNICODE
)
add_executable(${PROJECT_NAME} WIN32 ${SRCS})
endif()

add_dependencies(${PROJECT_NAME} runtime)
install(TARGETS ${PROJECT_NAME} DESTINATION ${MAIN_INST})
