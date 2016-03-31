#====================================================================#
# ASK signal pulse length histogram
#====================================================================#
generate_arduino_firmware(
	RF_analyser
    SKETCH RF_analyser
    NO_AUTOLIBS)

#====================================================================#
# Print serial input to a LCD1602 screen
#====================================================================#
generate_arduino_firmware(
	LCDConsole
    SKETCH LCDConsole
    LIBS LCD1602 BasicIO AvrTL
    NO_AUTOLIBS)

#====================================================================#
# Print an incresaing counter on a LCD1602 screen
#====================================================================#
generate_avr_firmware(
	LCDTest
    SRCS LCDTest.cpp
    LIBS LCD1602 BasicIO AvrTL AvrApp
    )

#====================================================================#
# Print an incresaing counter on a LCD1602 screen
#====================================================================#
generate_avr_firmware(
	LCD12864Test
    SRCS LCD12864Test.cpp
    LIBS LCD12864 AvrTL AvrApp
    )


#====================================================================#
# Simple input/output test for HW Serial port
#====================================================================#
generate_avr_firmware(
	HWSerialTest
    SRCS HWSerialTest.cpp
    LIBS BasicIO AvrApp
    )

#====================================================================#
# Detect and decode *any* IR/RF remote signals
#====================================================================#
generate_avr_firmware(
	Sniffer
	SRCS SnifferTest.cpp
	LIBS RFSniffer LCD1602 BasicIO AvrTL AvrApp
	)

#====================================================================#
# RF remote controlled car (from a DI-O/Chacon remote)
#====================================================================#
generate_avr_firmware(
	RFCar
	SRCS RFCar.cpp
	LIBS RFSniffer BasicIO AvrTL AvrApp
	)

#====================================================================#
# HC-SR04 / US-015 ultrasonic distance estimation
#====================================================================#
generate_avr_firmware(
	UltraSoundDistance
	SRCS UltraSoundDistance.cpp
	LIBS BasicIO AvrTL AvrApp
	)

#====================================================================#
# HC-SR04 / US-015 ultrasonic distance output as a pwm signal
#====================================================================#
generate_avr_firmware(
	UltraSoundDistancePWM
	SRCS UltraSoundDistancePWM.cpp
	LIBS TimeScheduler AvrTL AvrApp
	)

#====================================================================#
# Keypad test
#====================================================================#
generate_avr_firmware(
	keypad
	SRCS keypad.cpp
	LIBS BasicIO AvrTL AvrApp
	)

#====================================================================#
# SoftSerial test
#====================================================================#
generate_avr_firmware(
	SoftSerialTest
	SRCS SoftSerialTest.cpp
	LIBS SoftSerial BasicIO TimeScheduler AvrTL AvrApp
	)
