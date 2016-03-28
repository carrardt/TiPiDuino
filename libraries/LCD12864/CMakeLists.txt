#====================================================================#
# LCD1602 interface. compatible with ByteStream interface, may be used
# with a PrintStream
#====================================================================#
generate_arduino_library(
	LCD12864
	SRCS LCD12864.cpp
    HDRS LCD12864.h
    LIBS BasicIO AvrTL
    NO_AUTOLIBS MANUAL	
	)
