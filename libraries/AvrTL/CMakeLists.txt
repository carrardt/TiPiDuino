#====================================================================#
# AVR Template Library, low-level avr access, no Arduino/Wire dependency
#====================================================================#
generate_arduino_library(
	AvrTL
    SRCS AvrTLPin.cpp
    HDRS AvrTL.h AvrTLPin.h AvrTLEEPROM.h timer.h
    NO_AUTOLIBS MANUAL
	)

