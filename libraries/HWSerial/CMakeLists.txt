#====================================================================#
# ATmega328 USART0 interface. compatible with ByteStream interface, may be used
# with a PrintStream
#====================================================================#
generate_arduino_library(
	HWSerial
	SRCS HWSerialIO.cpp
    HDRS HWSerialIO.h
    LIBS BasicIO AvrTL
    NO_AUTOLIBS MANUAL
	)
