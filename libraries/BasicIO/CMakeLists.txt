#====================================================================#
# LCD1602 interface. compatible with ByteStream interface, may be used
# with a PrintStream
#====================================================================#
generate_arduino_library(
	BasicIO
	SRCS PrintStream.cpp
    HDRS ByteStream.h InputStream.h PrintStream.h
    LIBS AvrTL
    NO_AUTOLIBS MANUAL
	)
