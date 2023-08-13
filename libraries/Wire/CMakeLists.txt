#====================================================================#
# I2C Wire lib
#====================================================================#
generate_arduino_library(
	WireI2C
	SRCS Wire.cpp twi.cpp
	HDRS Wire.h twi.h
    LIBS AvrTL
    NO_AUTOLIBS MANUAL
    )

