#====================================================================#
# LCD1602 interface. compatible with ByteStream interface, may be used
# with a PrintStream
#====================================================================#
generate_arduino_library(
	KeyPad
	SRCS keypad.cpp
    HDRS keypad.h
    LIBS AvrTL
    NO_AUTOLIBS MANUAL
	)

