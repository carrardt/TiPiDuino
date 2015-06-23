cmake_minimum_required (VERSION 2.8)
project (Fleye)

# ./fleye -v -ex fixedfps -w 1296 -h 972 -p 0,0,648,486 --ccmd 4 --disptrack
# 
# 60+ fps configs
# ./fleye -v -ex fixedfps -w 1296 -h 972 -p 0,0,384,256 --ccmd 5 --disptrack 

SET(COMPILE_DEFINITIONS "-Werror")

SET(VCROOT "/opt/vc")
SET(VCINC "${VCROOT}/include")
SET(VCLIB "${VCROOT}/lib")

file(GLOB GLSL_SOURCE_FILES "*.glsl")
foreach(GLSL_SRC ${GLSL_SOURCE_FILES})
	get_filename_component(GLSL_SRC_NODIR ${GLSL_SRC} NAME)
	message("copy ${GLSL_SRC} to ${CMAKE_CURRENT_BINARY_DIR}/${GLSL_SRC_NODIR}")
	configure_file( ${GLSL_SRC} ${CMAKE_CURRENT_BINARY_DIR}/${GLSL_SRC_NODIR} COPYONLY)
endforeach(GLSL_SRC)


include_directories(${VCINC} ${VCINC}/interface/vcos/pthreads ${VCINC}/interface/vmcs_host/linux)
#add_definitions(-DGLSL_SRC_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\" -pthread)
add_definitions(-DGLSL_SRC_DIR=\".\" -pthread)

link_directories(${VCLIB})

add_library(gpio gpio.cpp bcm2835.c)

add_executable(fleye Fleye.c gl_tracking.c cpu_tracking.c RaspiCamControl.c RaspiCLI.c RaspiPreview.c RaspiTex.c RaspiTexUtil.c l2CrossObjectCenter.c )
add_executable(test_gpio test_gpio.c)

set (MMAL_LIBS mmal_core mmal_util mmal_vc_client)

target_link_libraries(test_gpio gpio)
target_link_libraries(fleye ${MMAL_LIBS} vcos bcm_host GLESv2 EGL pthread m gpio)

install(TARGETS fleye RUNTIME DESTINATION bin)

