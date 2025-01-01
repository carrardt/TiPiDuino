#====================================================================#
# AVR main definition, setup and loop functions declaration.
# To be used for pure avr (no Wiring/Arduino) apps
#====================================================================#
generate_avr_library(
	AvrApp
    SRCS AvrApp.cpp
    HDRS AvrApp.h
    ALL_BOARDS
	)
