#====================================================================#
# LCD1602 interface. compatible with ByteStream interface, may be used
# with a PrintStream
#====================================================================#
include_directories(../AvrTL)
generate_arduino_library(
	BasicIO
	SRCS PrintStream.cpp HWSerialIO.cpp
    HDRS ByteStream.h InputStream.h PrintStream.h SoftSerialIO.h HWSerialIO.h
    LIBS AvrTL
    NO_AUTOLIBS MANUAL
	)
