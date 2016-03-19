#====================================================================#
# AVR main definition, setup and loop functions declaration.
# To be used for pure avr (no Wiring/Arduino) apps
#====================================================================#
generate_arduino_library(
	AvrApp
    SRCS AvrApp.cpp
    HDRS AvrApp.h
    NO_AUTOLIBS MANUAL
	)
