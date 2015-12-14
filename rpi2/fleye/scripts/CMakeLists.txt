# copy scripts files
file(GLOB_RECURSE SCRIPTS_FILES "*.fleye")
foreach(SCRIPT ${SCRIPTS_FILES})
	get_filename_component(SCRIPT_NODIR ${SCRIPT} NAME)
	#message("copy ${SCRIPT} to ${CMAKE_CURRENT_BINARY_DIR}/${SCRIPT_NODIR}")
	configure_file( ${SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/${SCRIPT_NODIR} COPYONLY)
endforeach(SCRIPT)

