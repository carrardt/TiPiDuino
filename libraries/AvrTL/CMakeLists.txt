#====================================================================#
# AVR Template Library, low-level avr access, no Arduino/Wire dependency
#====================================================================#
generate_avr_library(
	AvrTL
    SRCS AvrTLPin.cpp
    HDRS AvrTL.h AvrTLPin.h AvrTLEEPROM.h timer.h
    ALL_BOARDS
	)

