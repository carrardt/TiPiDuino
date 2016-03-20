#====================================================================#
# RFSniffer. Infrared and RF signal analysis and decoding tools
#====================================================================#
generate_arduino_library(
	RFSniffer
	SRCS RFSnifferEEPROM.cpp RFSnifferProtocol.cpp
    HDRS RFSnifferConstants.h RFSnifferEEPROM.h RFSniffer.h RFSnifferInterpreter.h RFSnifferProtocol.h
    LIBS AvrTL BasicIO
    NO_AUTOLIBS MANUAL )
