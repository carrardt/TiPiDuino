# compile plugin dynamic libraries

include_directories(.. ../include ${GL_INC_DIR})

file(GLOB PLUGIN_SOURCE_FILES "*.cc")
foreach(PLUGIN_SRC ${PLUGIN_SOURCE_FILES})
	get_filename_component(PLUGIN_SRC_NODIR ${PLUGIN_SRC} NAME)
	get_filename_component(PLUGIN_NAME ${PLUGIN_SRC_NODIR} NAME_WE)
	add_library(${PLUGIN_NAME} SHARED ${PLUGIN_SRC})
	target_link_libraries(${PLUGIN_NAME} thirdparty)
	install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION bin)
endforeach(PLUGIN_SRC)

