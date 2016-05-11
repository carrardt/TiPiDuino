#====================================================================#
# RF remote controlled car (from a DI-O/Chacon remote)
#====================================================================#
generate_avr_firmware(
	RFCar
	SRCS RFCar.cpp
	LIBS RFSniffer BasicIO AvrTL AvrApp
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
# SoftSerial test
#====================================================================#
generate_avr_firmware(
	SoftSerialTest
	SRCS SoftSerialTest.cpp
	LIBS SoftSerial BasicIO TimeScheduler AvrTL AvrApp
	)

#====================================================================#
# FastSerial test
#====================================================================#
generate_avr_firmware(
	FastSerialTest
	SRCS FastSerialTest.cpp
	LIBS FastSerial AvrTL AvrApp
	)

#====================================================================#
# FastSerial receive test
#====================================================================#
generate_avr_firmware(
	FastSerialRead
	SRCS FastSerialRead.cpp
	LIBS FastSerial SoftSerial TimeScheduler AvrTL AvrApp
	)

#====================================================================#
# Clock test
#====================================================================#
generate_avr_firmware(
	TestFreq
	SRCS TestFreq.cpp
	LIBS AvrApp
	)

#====================================================================#
# Clock test using AvrTL pins
#====================================================================#
generate_avr_firmware(
	TestFreqAvrTL
	SRCS TestFreqAvrTL.cpp
	LIBS AvrApp
	)

#====================================================================#
# DC motor controller, taking inputs from soft-serial input
#====================================================================#
generate_avr_firmware(
	DCMotorController
	SRCS DCMotorController.cpp
	LIBS SoftSerial AvrTL AvrApp
	)

if(NOT "${ARDUINO_DEFAULT_BOARD}" MATCHES "attiny85.*" )
#====================================================================#
# Keypad test
#====================================================================#
generate_avr_firmware(
	keypad
	SRCS keypad.cpp
	LIBS HWSerial BasicIO AvrTL AvrApp
	)

#====================================================================#
# HC-SR04 / US-015 ultrasonic distance estimation
#====================================================================#
generate_avr_firmware(
	UltraSoundDistance
	SRCS UltraSoundDistance.cpp
	LIBS HWSerial BasicIO AvrTL AvrApp
	)


#====================================================================#
# Detect and decode *any* IR/RF remote signals
#====================================================================#
generate_avr_firmware(
	Sniffer
	SRCS SnifferTest.cpp
	LIBS RFSniffer LCD1602 HWSerial BasicIO AvrTL AvrApp
	)

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
	LCD12864Console
    SRCS LCD12864Console.cpp
    LIBS LCD12864 SoftSerial BasicIO AvrTL AvrApp
    )
    
#====================================================================#
# read values from serial and generate several pwm signals
#====================================================================#
generate_avr_firmware(
	LinkuinoServer
	SRCS LinkuinoServer.cpp
	LIBS HWSerialNoInt TimeScheduler Linkuino AvrTL AvrApp
	)

#====================================================================#
# Print an incresaing counter on a LCD1602 screen
#====================================================================#
generate_avr_firmware(
	LCD12864Test
    SRCS LCD12864Test.cpp
    LIBS LCD12864 BasicIO AvrTL AvrApp
    )

#====================================================================#
# Simple input/output test for HW Serial port
#====================================================================#
generate_avr_firmware(
	HWSerialTest
    SRCS HWSerialTest.cpp
    LIBS HWSerial BasicIO AvrApp
    )

#====================================================================#
# Simple input/output test for HW Serial port not using interrupts
#====================================================================#
generate_avr_firmware(
	HWSerialNoIntTest
    SRCS HWSerialNoIntTest.cpp
    LIBS HWSerialNoInt BasicIO AvrApp
    )
endif()
