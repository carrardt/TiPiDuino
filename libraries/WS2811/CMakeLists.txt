#====================================================================#
# LCD1602 interface. compatible with ByteStream interface, may be used
# with a PrintStream
#====================================================================#
generate_arduino_library(
	WS2811
	SRCS ws2811.cpp
    HDRS ws2811.h
    LIBS AvrTL
    NO_AUTOLIBS MANUAL
	)

