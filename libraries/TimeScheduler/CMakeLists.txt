#====================================================================#
# LCD1602 interface. compatible with ByteStream interface, may be used
# with a PrintStream
#====================================================================#
generate_avr_library(
	TimeScheduler
    HDRS TimeScheduler.h
    LIBS AvrTL
    ALL_BOARDS
	)
