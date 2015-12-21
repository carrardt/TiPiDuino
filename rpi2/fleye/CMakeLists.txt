cmake_minimum_required (VERSION 2.8)
project (Fleye)

# various usage samples
# 
# ./fleye -v -ex fixedfps -w 1296 -h 972 -p 32,32,800,544 --script wincoord
# ./fleye -v -ex fixedfps -w 1296 -h 972 -p 32,32,800,544 --script normcoord
# ./fleye -v -ex fixedfps -w 1296 -h 972 -p 32,32,352,272 --script wincoord
# ./fleye -v -ex fixedfps -w 1296 -h 972 -p 32,32,352,272 --script normcoord
# sudo ./fleye -v -ex fixedfps -drc off -w 1296 -h 972 -p 0,0,384,256 --set "CCMD 5" --script l2centerfollow 
# ./RaspiCore/fleye -v -ex fixedfps -drc off -w 2592 -h 1944 -p 0,0,768,512 --script simplefbo
# ./RaspiCore/fleye -v -ex fixedfps -drc off -w 2592 -h 1944 -p 0,0,768,512 --script l2cross


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
SET(SCRIPTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/scripts )
SET(PLUGINS_DIR ${CMAKE_CURRENT_BINARY_DIR}/plugins )

add_subdirectory(thirdparty)
add_subdirectory(gpiotest)
add_subdirectory(plugins)
add_subdirectory(scripts)
add_subdirectory(shaders)

add_subdirectory(RaspiCam)
add_subdirectory(TestCamera)

find_package(Jsoncpp REQUIRED)
add_subdirectory(FleyeModule)
add_subdirectory(RaspiCore)
