generate_avr_firmware(avrtl_blink SRCS avrtl_blink.cpp LIBS AvrTL BOARDS nano)
generate_avr_firmware(StepMotorTest SRCS StepMotorTest.cpp LIBS SoftSerial BasicIO AvrTL ALL_BOARDS)
generate_avr_firmware(TestPCD8544 SRCS TestPCD8544.cpp LIBS PCD8544 AvrTL ALL_BOARDS)

