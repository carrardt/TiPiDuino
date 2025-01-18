#====================================================================#
# I2C Wire lib
#====================================================================#
generate_arduino_library(
	WireI2C
	SRCS WireI2C.cpp twi.cpp
	HDRS WireI2C.h twi.h
    LIBS AvrTL
    NO_AUTOLIBS MANUAL
    )

