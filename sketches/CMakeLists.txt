generate_avr_firmware(avrtl_blink SRCS avrtl_blink.cpp LIBS AvrTL BOARDS nano)
generate_avr_firmware(StepMotorTest SRCS StepMotorTest.cpp LIBS SoftSerial BasicIO AvrTL ALL_BOARDS)

