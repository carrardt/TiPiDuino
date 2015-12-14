file(GLOB FLEYE_SRC "*.cc")

include_directories(../include ${GL_INC_DIR})
add_definitions(-DGLSL_SRC_DIR=\"${GLSL_SHADER_DIR}\" -DFLEYE_PLUGIN_DIR=\"${PLUGINS_DIR}\" -DFLEYE_SCRIPT_DIR=\"${SCRIPTS_DIR}\")

add_library(FleyeModule ${FLEYE_SRC})

target_link_libraries(FleyeModule GLESv2 EGL m dl)

