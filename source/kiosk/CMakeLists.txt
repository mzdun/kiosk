if (UNIX)
set(LAUNCHER_DIRSEP "/")
set(LAUNCHER_PATHSEP ":")
else()
set(LAUNCHER_DIRSEP "\\\\")
set(LAUNCHER_PATHSEP ";")
endif()

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/paths_kiosk.h.in ${CMAKE_CURRENT_BINARY_DIR}/paths_kiosk.h)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

if (UNIX)
set(POSTFIX "linux")
elseif (WIN32)
set(POSTFIX "win")
endif()

set(SRCS main_kiosk_${POSTFIX}.cc)

if (UNIX)
add_executable(kiosk ${SRCS})
target_link_libraries(kiosk ${CMAKE_DL_LIBS})
elseif (WIN32)
add_definitions(-DUNICODE -D_UNICODE)
list(APPEND SRCS kiosk.rc)
add_executable(kiosk WIN32 ${SRCS})
endif()

add_dependencies(kiosk runtime)
install(TARGETS kiosk DESTINATION ${MAIN_INST})
