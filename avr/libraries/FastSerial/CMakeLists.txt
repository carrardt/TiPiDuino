#====================================================================#
# LCD1602 interface. compatible with ByteStream interface, may be used
# with a PrintStream
#====================================================================#
generate_arduino_library(
	FastSerial
	SRCS FastSerial.cpp
    HDRS FastSerial.h
    LIBS TimeScheduler AvrTL
    NO_AUTOLIBS MANUAL
	)
