cmake_minimum_required(VERSION 3.4)
project(pphue CXX)

set(DEPS ppapi_cpp)


CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/version.h.in ${PROJECT_BINARY_DIR}/version.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/hue)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../libs/nacl/include)

set(SRC
	pp/instance.cc
	pp/instance.h
	pp/module.cc
)

add_library(${PROJECT_NAME} SHARED ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES
	CXX_STANDARD 11
	VERSION ${TOPLEVEL_VERSION}
	SOVERSION ${TOPLEVEL_VERSION_MAJOR}
	FOLDER libs)

foreach(DEP ${DEPS})
	add_dependencies(${PROJECT_NAME} ${DEP})
	target_link_libraries(${PROJECT_NAME} $<TARGET_FILE:${DEP}>)
endforeach()

install(TARGETS ${PROJECT_NAME}
	DESTINATION ${LIB_ROOT}plugins)
