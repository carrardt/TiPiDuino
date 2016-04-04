#====================================================================#
# ATmega328 USART0 low level interface. no interrupts.
#====================================================================#
generate_arduino_library(
	HWSerialNoInt
	SRCS HWSerialNoInt.cpp
    HDRS HWSerialNoInt.h
    NO_AUTOLIBS MANUAL
	)
