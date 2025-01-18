#====================================================================#
# ATmega328 USART0 low level interface. no interrupts.
#====================================================================#
generate_arduino_library(
	Linkuino
	SRCS Linkuino.cpp
    HDRS Linkuino.h
    NO_AUTOLIBS MANUAL
	)
