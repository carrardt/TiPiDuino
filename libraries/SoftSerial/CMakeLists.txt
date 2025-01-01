#====================================================================#
# LCD1602 interface. compatible with ByteStream interface, may be used
# with a PrintStream
#====================================================================#
generate_avr_library(
  SoftSerial
    HDRS SoftSerial.h
    LIBS TimeScheduler AvrTL
    ALL_BOARDS
  )

