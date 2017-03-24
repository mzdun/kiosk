add_custom_target(kiosk-apps
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/default" "${CMAKE_CURRENT_BINARY_DIR}/../apps/default"
	VERBATIM
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/default DESTINATION ${APPS_INST})
