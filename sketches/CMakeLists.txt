
#====================================================================#
# Step motor test
#====================================================================#
generate_avr_firmware(
	StepMotorTest
	SRCS StepMotorTest.cpp
	LIBS SoftSerial BasicIO AvrTL AvrApp
	ALL_BOARDS
	)

