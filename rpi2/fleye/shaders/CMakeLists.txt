# copy shader files
file(GLOB_RECURSE GLSL_SOURCE_FILES "*.glsl")
foreach(GLSL_SRC ${GLSL_SOURCE_FILES})
	get_filename_component(GLSL_SRC_NODIR ${GLSL_SRC} NAME)
	#message("copy ${GLSL_SRC} to ${CMAKE_CURRENT_BINARY_DIR}/${GLSL_SRC_NODIR}")
	configure_file( ${GLSL_SRC} ${CMAKE_CURRENT_BINARY_DIR}/${GLSL_SRC_NODIR} COPYONLY)
endforeach(GLSL_SRC)

