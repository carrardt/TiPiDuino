#====================================================================#
# AVR Template Library, low-level avr access, no Arduino/Wire dependency
#====================================================================#
generate_arduino_library(
	AvrTL
    SRCS AvrTLPin.cpp AvrTLSignal.cpp
    HDRS AvrTL.h AvrTLPin.h AvrTLEEPROM.h AvrTLSignal.h
    NO_AUTOLIBS MANUAL
	)
