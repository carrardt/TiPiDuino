#====================================================================#
# LCD1602 interface. compatible with ByteStream interface, may be used
# with a PrintStream
#====================================================================#
generate_arduino_library(
	TimeScheduler
	SRCS TimeScheduler.cpp
    HDRS TimeScheduler.h
    LIBS AvrTL
    NO_AUTOLIBS MANUAL
	)
