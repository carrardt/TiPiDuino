include_directories(../RaspiCam ${VCINC} ${VCINC}/interface/vcos/pthreads ${VCINC}/interface/vmcs_host/linux)
add_definitions(-pthread)
link_directories(${VCLIB})

file(GLOB TESTCAMERA_SRC "*.c")
add_executable(picam ${TESTCAMERA_SRC})
target_link_libraries(picam RaspiCam ${MMAL_LIBS} vcos bcm_host GLESv2 EGL pthread m dl)
install(TARGETS picam RUNTIME DESTINATION bin)
