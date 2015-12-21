include_directories(../include ${VCINC} ${VCINC}/interface/vcos/pthreads ${VCINC}/interface/vmcs_host/linux)
add_definitions(-pthread)
link_directories(${VCLIB})

file(GLOB RASPICAM_SRC "*.c")
add_library(RaspiCam ${RASPICAM_SRC})

