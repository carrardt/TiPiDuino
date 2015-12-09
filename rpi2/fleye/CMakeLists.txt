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

SET(COMPILE_DEFINITIONS "-Werror")

SET(VCROOT "/opt/vc")
SET(VCINC "${VCROOT}/include")
SET(VCLIB "${VCROOT}/lib")
set (MMAL_LIBS mmal_core mmal_util mmal_vc_client)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${VCINC} ${VCINC}/interface/vcos/pthreads ${VCINC}/interface/vmcs_host/linux)
add_definitions(-DGLSL_SRC_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\" -pthread)
link_directories(${VCLIB})

SET(FLEYE_SRC
	src/Fleye.c
	src/glworker.c
	src/cpuworker.c
	src/RaspiCamControl.c
	src/RaspiCLI.c
	src/RaspiPreview.c
	src/fleye_core.c
	src/fleye_util.c )

SET(TESTGPIO_SRC
	src/test_gpio.c
	thirdparty/bcm2835.c )

add_executable(fleye ${FLEYE_SRC})
target_link_libraries(fleye ${MMAL_LIBS} vcos bcm_host GLESv2 EGL pthread m dl)

add_executable(test_gpio ${TESTGPIO_SRC})

# copy shader files
file(GLOB_RECURSE GLSL_SOURCE_FILES "*.glsl")
foreach(GLSL_SRC ${GLSL_SOURCE_FILES})
	get_filename_component(GLSL_SRC_NODIR ${GLSL_SRC} NAME)
	message("copy ${GLSL_SRC} to ${CMAKE_CURRENT_BINARY_DIR}/${GLSL_SRC_NODIR}")
	configure_file( ${GLSL_SRC} ${CMAKE_CURRENT_BINARY_DIR}/${GLSL_SRC_NODIR} COPYONLY)
endforeach(GLSL_SRC)

# copy scripts files
file(GLOB_RECURSE SCRIPTS_FILES "*.fleye")
foreach(SCRIPT ${SCRIPTS_FILES})
	get_filename_component(SCRIPT_NODIR ${SCRIPT} NAME)
	message("copy ${SCRIPT} to ${CMAKE_CURRENT_BINARY_DIR}/${SCRIPT_NODIR}")
	configure_file( ${SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/${SCRIPT_NODIR} COPYONLY)
endforeach(SCRIPT)


# compile plugin dynamic libraries
file(GLOB PLUGIN_SOURCE_FILES "plugins/*.c")
foreach(PLUGIN_SRC ${PLUGIN_SOURCE_FILES})
	get_filename_component(PLUGIN_SRC_NODIR ${PLUGIN_SRC} NAME)
	get_filename_component(PLUGIN_NAME ${PLUGIN_SRC_NODIR} NAME_WE)
	add_library(${PLUGIN_NAME} SHARED ${PLUGIN_SRC})
	install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION bin)
endforeach(PLUGIN_SRC)

install(TARGETS fleye RUNTIME DESTINATION bin)

