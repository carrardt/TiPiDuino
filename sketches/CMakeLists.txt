
#====================================================================#
# Step motor test
#====================================================================#
generate_avr_firmware(
	StepMotorTest
	SRCS StepMotorTest.cpp
	LIBS BasicIO AvrTL AvrApp
	BOARDS nano attiny84
	)

