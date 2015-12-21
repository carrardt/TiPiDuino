include_directories(../include ../RaspiCam ${VCINC} ${VCINC}/interface/vcos/pthreads ${VCINC}/interface/vmcs_host/linux)
add_definitions(-pthread)
link_directories(${VCLIB})

file(GLOB RASPICORE_SRC "*.c")
add_executable(fleye ${RASPICORE_SRC})
# message("Json lib is ${jsoncpp_LIBRARIES}")
target_link_libraries(fleye RaspiCam FleyeModule ${jsoncpp_LIBRARIES} thirdparty ${MMAL_LIBS} vcos bcm_host GLESv2 EGL pthread m dl)
install(TARGETS fleye RUNTIME DESTINATION bin)

