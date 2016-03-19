#====================================================================#
# LCD1602 interface. compatible with ByteStream interface, may be used
# with a PrintStream
#====================================================================#
include_directories(../AvrTL ../BasicIO)
generate_arduino_library(
	LCD1602
	SRCS LCD1602.cpp
    HDRS LCD1602.h
    LIBS BasicIO
    NO_AUTOLIBS MANUAL
	)
