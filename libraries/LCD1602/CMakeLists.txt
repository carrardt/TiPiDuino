#====================================================================#
# LCD1602 interface. compatible with ByteStream interface, may be used
# with a PrintStream
#====================================================================#
generate_arduino_library(
	LCD1602
	SRCS LCD1602.cpp
    HDRS LCD1602.h
    LIBS BasicIO AvrTL
    NO_AUTOLIBS MANUAL
	)
