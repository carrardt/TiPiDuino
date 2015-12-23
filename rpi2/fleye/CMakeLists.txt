cmake_minimum_required (VERSION 2.8)
project (Fleye)

# various usage samples
# 
# ./Fleye/fleye -v -ex fixedfps -res 1280x960 -geom 640x480+32+32 -script wincoord
# optimal resolution/performance
# ./Fleye/fleye -ex fixedfps -res 1280x960 -geom 1024x768 -v
# ./Fleye/fleye -ex fixedfps -res 1280x960 -geom 640x480 -script camera-rgba


# Raspberry Pi VideoCore HW Abstraction Layer
SET(VCROOT "/opt/vc")
SET(VCINC "${VCROOT}/include")
SET(VCLIB "${VCROOT}/lib")
SET(MMAL_LIBS mmal_core mmal_util mmal_vc_client)

# OpenGL ES headers from userland code
SET(GL_INC_DIR ${VCINC} ${VCINC}/interface/vcos/pthreads)
SET(GL_LIB_DIR "${VCLIB}")

# Resource directories
SET(GLSL_SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
SET(SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scripts)
SET(PLUGINS_DIR ${CMAKE_CURRENT_BINARY_DIR}/plugins)
SET(DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)

add_subdirectory(thirdparty)
add_subdirectory(gpiotest)
add_subdirectory(plugins)
add_subdirectory(scripts)
add_subdirectory(shaders)

add_subdirectory(RaspiCam)
add_subdirectory(TestCamera)

find_package(Jsoncpp REQUIRED)
add_subdirectory(Fleye)

