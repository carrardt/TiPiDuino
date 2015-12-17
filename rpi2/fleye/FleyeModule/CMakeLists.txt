file(GLOB FLEYE_SRC "*.cc")

#add_compile_options(-std=c++11) # in cmake 2.8.12 and later
add_definitions(-std=c++11)

add_definitions(-DGLSL_SRC_DIR=\"${GLSL_SHADER_DIR}\" -DFLEYE_PLUGIN_DIR=\"${PLUGINS_DIR}\" -DFLEYE_SCRIPT_DIR=\"${SCRIPTS_DIR}\")
include_directories(../include ${GL_INC_DIR} ${Jsoncpp_INCLUDE_DIR})

add_library(FleyeModule ${FLEYE_SRC})
