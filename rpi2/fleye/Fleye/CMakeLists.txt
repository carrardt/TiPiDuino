include_directories(../include ../RaspiCam ${GL_INC_DIR} ${Jsoncpp_INCLUDE_DIR} ${VCINC} ${VCINC}/interface/vcos/pthreads ${VCINC}/interface/vmcs_host/linux)
add_definitions(-pthread)
link_directories(${VCLIB})

file(GLOB FLEYE_SRC "*.c" "*.cc")

#add_compile_options(-std=c++11) # in cmake 2.8.12 and later
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_definitions(-DGLSL_SRC_DIR=\"${GLSL_SHADER_DIR}\" -DFLEYE_PLUGIN_DIR=\"${PLUGINS_DIR}\" -DFLEYE_SCRIPT_DIR=\"${SCRIPTS_DIR}\")
include_directories(../include ${GL_INC_DIR} ${Jsoncpp_INCLUDE_DIR})

add_executable(fleye ${FLEYE_SRC})
target_link_libraries(fleye RaspiCam ${jsoncpp_LIBRARIES} ${MMAL_LIBS} vcos bcm_host GLESv2 EGL pthread m dl)
install(TARGETS fleye RUNTIME DESTINATION bin)
