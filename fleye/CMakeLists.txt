
# raspistill/raspivid/raspiyuv

SET(COMPILE_DEFINITIONS -Werror)

set(RASPICAM_SOURCE_DIR "${PROJECT_SOURCE_DIR}/host_applications/linux/apps/raspicam" )

include_directories(${PROJECT_SOURCE_DIR}/host_applications/linux/libs/bcm_host/include)
include_directories("${RASPICAM_SOURCE_DIR}")

add_definitions(-DGLSL_SRC_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")

set(RASPICAM_SOURCES
   ${RASPICAM_SOURCE_DIR}/RaspiCamControl.c
   ${RASPICAM_SOURCE_DIR}/RaspiCLI.c
   ${RASPICAM_SOURCE_DIR}/RaspiPreview.c
   ${RASPICAM_SOURCE_DIR}/RaspiTexUtil.c
   ${RASPICAM_SOURCE_DIR}/tga.c
)

add_executable(fleye Fleye.c GPUTracking.c gl_tracking.c cpu_tracking.c ${RASPICAM_SOURCES} )

set (MMAL_LIBS mmal_core mmal_util mmal_vc_client)

target_link_libraries(fleye ${MMAL_LIBS} vcos bcm_host GLESv2 EGL m)

install(TARGETS fleye RUNTIME DESTINATION bin)

