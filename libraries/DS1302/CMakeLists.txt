#====================================================================#
# LCD1602 interface. compatible with ByteStream interface, may be used
# with a PrintStream
#====================================================================#
generate_arduino_library(
	DS1302
	SRCS Ds1302.cpp
	HDRS Ds1302.h
    LIBS AvrTL
    NO_AUTOLIBS MANUAL
	)

