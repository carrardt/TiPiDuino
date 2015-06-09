cmake_minimum_required (VERSION 2.8)
project (Fleye)

# ./fleye -v -ex fixedfps -w 1296 -h 972 -p 0,0,648,486 --ccmd 4 --disptrack

SET(COMPILE_DEFINITIONS "-Werror")

SET(VCROOT "/opt/vc")
SET(VCINC "${VCROOT}/include")
SET(VCLIB "${VCROOT}/lib")

include_directories(${VCINC} ${VCINC}/interface/vcos/pthreads ${VCINC}/interface/vmcs_host/linux)
add_definitions(-DGLSL_SRC_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\" -pthread)

link_directories(${VCLIB})

add_executable(fleye Fleye.c gl_tracking.c cpu_tracking.c RaspiCamControl.c RaspiCLI.c RaspiPreview.c RaspiTex.c RaspiTexUtil.c )

set (MMAL_LIBS mmal_core mmal_util mmal_vc_client)

target_link_libraries(fleye ${MMAL_LIBS} vcos bcm_host GLESv2 EGL pthread m)

install(TARGETS fleye RUNTIME DESTINATION bin)

