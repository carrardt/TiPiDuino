#====================================================================#
# Pin test through serial port
#====================================================================#
generate_avr_firmware(
	SerialPinTest
	SRCS SerialPinTest.cpp
	LIBS SoftSerial BasicIO AvrTL AvrApp
	)

#====================================================================#
# Step motor test
#====================================================================#
generate_avr_firmware(
	StepMotorTest
	SRCS StepMotorTest.cpp
	LIBS BasicIO AvrTL AvrApp
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
# DC motor controller, taking inputs from soft-serial or fast-serial input
#====================================================================#
generate_avr_firmware(
	DCMotorController
	SRCS DCMotorController.cpp
	LIBS SoftSerial AvrTL AvrApp
	)

#====================================================================#
# Serial input to PCD8544 LCD screen
#====================================================================#
generate_avr_firmware(
	SerialToPCD8544
    SRCS SerialToPCD8544.cpp
    LIBS PCD8544 BasicIO AvrTL AvrApp
    )


if(NOT "${ARDUINO_DEFAULT_BOARD}" MATCHES "attiny85.*")
#====================================================================#
# Self shutting down relay timer
#====================================================================#
# workd with atmega328 and attiny84
generate_avr_firmware(
	TimerRelaySelfShutdownSerial
	SRCS TimerRelaySelfShutdownSerial.cpp
	LIBS SoftSerial BasicIO TimeScheduler AvrTL AvrApp
	)
endif()

if("${ARDUINO_DEFAULT_BOARD}" MATCHES "attiny84.*")
#====================================================================#
# Calculator
#====================================================================#
generate_avr_firmware(
	Calculator
    SRCS Calculator.cpp
    LIBS PCD8544 BasicIO AvrTL AvrApp
    )
endif()

if(NOT "${ARDUINO_DEFAULT_BOARD}" MATCHES "attiny8.*")

#====================================================================#
# prints 0-99 numbers to serial port at 19200
#====================================================================#
generate_avr_firmware(
	SerialOutput
	SRCS SerialOutput.cpp
	LIBS HWSerial BasicIO AvrTL AvrApp
	)

#====================================================================#
# DC motor controller fast-serial input tester
#====================================================================#
generate_avr_firmware(
	DCMotorInputTester
	SRCS DCMotorInputTester.cpp
	LIBS FastSerial HWSerial AvrTL AvrApp
	)

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
# Print input from a PS2 keyboard to LCD1602 screen
#====================================================================#
generate_avr_firmware(
	PS2KbdLCD1602
    SRCS PSkeyboard_LCD1602.cpp
    LIBS LCD1602 BasicIO AvrTL AvrApp
    )

#====================================================================#
# Print fast-serial 24bits input values to a LCD1602 screen
#====================================================================#
generate_avr_firmware(
    FastSerialLCD1602
    SRCS FastSerialLCD1602.cpp
    LIBS FastSerial LCD1602 BasicIO AvrTL AvrApp
    NO_AUTOLIBS)

#====================================================================#
# Programmable timer wich triggers a relay, with an LCD1602 screen
#====================================================================#
generate_avr_firmware(
    TimerRelayLCD1602
    SRCS TimerRelayLCD1602.cpp
    LIBS LCD1602 BasicIO AvrTL AvrApp
    NO_AUTOLIBS)

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
	LIBS FastSerial HWSerialNoInt TimeScheduler Linkuino AvrTL AvrApp
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
# Soft Serial to HW Serial
#====================================================================#
generate_avr_firmware(
	SoftSerial2HWSerial
    SRCS SoftSerial2HWSerial.cpp
    LIBS HWSerial SoftSerial BasicIO AvrApp
    )

#====================================================================#
# Simple input/output test for HW Serial port not using interrupts
#====================================================================#
generate_avr_firmware(
	HWSerialNoIntTest
    SRCS HWSerialNoIntTest.cpp
    LIBS HWSerialNoInt BasicIO AvrApp
    )

#====================================================================#
# Hello World test for PCD8544 LCD screen (aka Nokia5110)
#====================================================================#
generate_avr_firmware(
	TestPCD8544
    SRCS TestPCD8544.cpp
    LIBS PCD8544 AvrTL AvrApp
    )
    
endif()
