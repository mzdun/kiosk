cmake_minimum_required(VERSION 3.4)
project(panelberry CXX)

set(CMAKE_MODULE_PATH
  ${PROJECT_SOURCE_DIR}/cmake
  ${CMAKE_MODULE_PATH})

set(TOPLEVEL_VERSION_MAJOR 0)
set(TOPLEVEL_VERSION_MINOR 1)
set(TOPLEVEL_VERSION_PATCH 0)
set(TOPLEVEL_VERSION_BUILD 1)
set(TOPLEVEL_VERSION_STABILITY "-alpha")
set(TOPLEVEL_VERSION ${TOPLEVEL_VERSION_MAJOR}.${TOPLEVEL_VERSION_MINOR}.${TOPLEVEL_VERSION_PATCH})

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/project_version.h.in ${PROJECT_BINARY_DIR}/project_version.h)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR})

if (UNIX)
set(SHARE_PREFIX "share/")
set(LIB_PREFIX   "lib/")

set(MAIN_INST   bin)                   # kiosk
set(SHARE_INST  share/kiosk)           # runtime
set(RES_INST    ${SHARE_INST}/resources) # .pak
set(LOCALE_INST ${SHARE_INST})         # locales
set(CEF_INST    lib/kiosk)             # libcef.so
set(DATA_INST   ${SHARE_INST})         # .blob, .dat
set(PLUGIN_INST lib/kiosk/plugins)     # libpphue.so
set(APPS_INST   share/kiosk/apps)      # kiosk:// apps

set(DEF_LIB_DST  ${CMAKE_INSTALL_PREFIX}/${CEF_INST})
set(DEF_RT_DST   ${CMAKE_INSTALL_PREFIX}/${SHARE_INST})
set(DEF_APPS_DST ${CMAKE_INSTALL_PREFIX}/${SHARE_INST}/apps)

set(KIOSK_CMAKE_BINARY_DIR   bin)
set(RUNTIME_CMAKE_BINARY_DIR ${SHARE_INST})
set(RESURCE_CMAKE_BINARY_DIR ${RES_INST})
set(LOCALE_CMAKE_BINARY_DIR  ${LOCALE_INST})
set(CEF_CMAKE_BINARY_DIR     ${CEF_INST})
set(CEFDATA_CMAKE_BINARY_DIR ${CEF_CMAKE_BINARY_DIR})
else()
set(SHARE_PREFIX "")
set(LIB_PREFIX   "")

set(MAIN_INST   ".")                   # kiosk
set(SHARE_INST  runtime)               # runtime
set(RES_INST    resources)             # .pak
set(LOCALE_INST ".")                   # locales
set(CEF_INST    runtime/lib)           # libcef.dll, gl, d3d
set(DATA_INST   ${CEF_INST})           # .blob, .dat
set(PLUGIN_INST plugins)               # pphue.dll
set(APPS_INST   apps)                  # kiosk:// apps

set(DEF_LIB_DST  runtime\\\\lib)
set(DEF_RT_DST   ${SHARE_INST})
set(DEF_APPS_DST ..\\\\apps)

set(KIOSK_CMAKE_BINARY_DIR   kiosk)
set(RUNTIME_CMAKE_BINARY_DIR kiosk/${SHARE_INST})
set(RESURCE_CMAKE_BINARY_DIR kiosk/${RES_INST})
set(CEF_CMAKE_BINARY_DIR     kiosk/${CEF_INST})
set(LOCALE_CMAKE_BINARY_DIR  kiosk)
set(CEFDATA_CMAKE_BINARY_DIR ${RUNTIME_CMAKE_BINARY_DIR})
endif()

set(RUNTIME_DST DATA_DST)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG   "")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG   "..")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE "..")

add_subdirectory(libs            lib) # to minimise chaos in binary dirs
add_subdirectory(plugins/pphue   ${LIB_PREFIX}kiosk/plugins/pphue)
add_subdirectory(apps/runtime    ${RUNTIME_CMAKE_BINARY_DIR})
add_subdirectory(apps/kiosk      ${KIOSK_CMAKE_BINARY_DIR})
add_subdirectory(apps/kiosk-apps ${SHARE_PREFIX}kiosk/apps)

set(CPACK_PACKAGE_VENDOR "midnightBITS")
set(CPACK_PACKAGE_CONTACT "panelberry-maintainers@midnightbits.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Control panel for IoT devices on RPi")
set(CPACK_PACKAGE_VERSION_MAJOR "${TOPLEVEL_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${TOPLEVEL_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${TOPLEVEL_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION "${TOPLEVEL_VERSION}${TOPLEVEL_VERSION_STABILITY}+${TOPLEVEL_VERSION_BUILD}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Kiosk ${TOPLEVEL_VERSION_MAJOR}.${TOPLEVEL_VERSION_MINOR}")
include(CPack)
