cmake_minimum_required (VERSION 2.8)
project (Fleye)

# ./fleye -v -ex fixedfps -w 1296 -h 972 -p 0,0,648,486 --ccmd 4 --disptrack
# 
# 60+ fps configs
# ./fleye -v -ex fixedfps -w 1296 -h 972 -p 0,0,384,256 --set "CCMD 5" --script l2crosscenter
#
# pixel coord precision tests
# ./fleye -v -ex fixedfps -w 1296 -h 972 -p 32,32,800,544 --script wincoord
# ./fleye -v -ex fixedfps -w 1296 -h 972 -p 32,32,800,544 --script normcoord
# ./fleye -v -ex fixedfps -w 1296 -h 972 -p 32,32,352,272 --script wincoord
# ./fleye -v -ex fixedfps -w 1296 -h 972 -p 32,32,352,272 --script normcoord
# sudo ./fleye -v -ex fixedfps -drc off -w 1296 -h 972 -p 0,0,384,256 --set "CCMD 5" --script l2centerfollow 

# Raspberry Pi VideoCore HW Abstraction Layer
SET(VCROOT "/opt/vc")
SET(VCINC "${VCROOT}/include")
SET(GL_INC_DIR ${VCINC} ${VCINC}/interface/vcos/pthreads)
SET(VCLIB "${VCROOT}/lib")
SET(GL_LIB_DIR "${VCLIB}")
SET(MMAL_LIBS mmal_core mmal_util mmal_vc_client)
SET(GLSL_SHADER_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
SET(SCRIPTS_DIR ${CMAKE_CURRENT_BINARY_DIR}/scripts )
SET(PLUGINS_DIR ${CMAKE_CURRENT_BINARY_DIR}/plugins )

add_subdirectory(thirdparty)
add_subdirectory(gpiotest)
add_subdirectory(plugins)
add_subdirectory(scripts)
add_subdirectory(shaders)

find_package(Jsoncpp REQUIRED)
add_subdirectory(FleyeModule)
add_subdirectory(RaspiCore)
