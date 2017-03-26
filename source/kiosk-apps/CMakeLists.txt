set(DEFAULT_DIR "${CMAKE_CURRENT_BINARY_DIR}/../apps/default")
add_custom_target(kiosk-apps
	COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/default" "${DEFAULT_DIR}"
	DEPENDS "${DEFAULT_DIR}/index.html"
	VERBATIM
)

add_custom_command(
	OUTPUT "${DEFAULT_DIR}/index.html"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
	COMMAND python "../../tools/make_template.py"
			"--version=${TOPLEVEL_VERSION_MAJOR}.${TOPLEVEL_VERSION_MINOR}.${TOPLEVEL_VERSION_PATCH}${TOPLEVEL_VERSION_STABILITY}+${TOPLEVEL_VERSION_BUILD}"
			"@${CMAKE_CURRENT_SOURCE_DIR}/index.html.txt"
			-o "${DEFAULT_DIR}/index.html"
	MAIN_DEPENDENCY "${CMAKE_CURRENT_SOURCE_DIR}/../../tools/make_template.py"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../res/kiosk.svg"
	COMMENT "Generating default index.html"
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/default DESTINATION ${APPS_INST})
install(DIRECTORY ${DEFAULT_DIR} DESTINATION ${APPS_INST})
