#====================================================================#
# LCD1602 interface. compatible with ByteStream interface, may be used
# with a PrintStream
#====================================================================#
generate_arduino_library(
	SoftSerial
	SRCS SoftSerial.cpp
    HDRS SoftSerial.h
    LIBS TimeScheduler AvrTL
    NO_AUTOLIBS MANUAL
	)
