#====================================================================#
# I2C Wire lib
#====================================================================#
generate_arduino_library(
	uRTCLib
	SRCS uRTCLib.cpp
	HDRS uRTCLib.h
	LIBS WireI2C
    NO_AUTOLIBS MANUAL
    )

