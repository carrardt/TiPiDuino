#====================================================================#
# AVR Template Library, low-level avr access, no Arduino/Wire dependency
#====================================================================#
add_subdirectory(AvrTL)
add_subdirectory(TimeScheduler)
add_subdirectory(SoftSerial)
add_subdirectory(FastSerial)
if(NOT "${ARDUINO_DEFAULT_BOARD}" MATCHES "attiny8.*" )
	add_subdirectory(HWSerial) # to be removed once port to noint version is done
	add_subdirectory(HWSerialNoInt)
endif()
add_subdirectory(AvrApp)
add_subdirectory(BasicIO)
add_subdirectory(LCD1602)
if(NOT "${ARDUINO_DEFAULT_BOARD}" MATCHES "attiny8.*" )
	add_subdirectory(LCD12864) # Needs to be ported for attiny compatibility
  add_subdirectory(WS2811)
endif()
add_subdirectory(RFSniffer)
add_subdirectory(Linkuino)
add_subdirectory(PCD8544)
add_subdirectory(DS1302)
add_subdirectory(WireI2C)
add_subdirectory(uRTCLib)

