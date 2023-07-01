# in case you want to manually upload a .hex to an avr, using an ArduinoAsISP, you can use these 
# shell functions :

export ARDUINOISP_SERIALSPEED=19200 # standard value as in the ArduinoISP original sketch
export ARDUINONANO_SERIALSPEED=57600 # standard value as in the ArduinoISP original sketch
export ATMEGA328_BOOT_IMG=/usr/share/arduino/hardware/arduino/avr/bootloaders/optiboot/optiboot_atmega328.hex 

tn85upload()
{
  avrdude -v -pattiny85 -cstk500v1 -P/dev/tty$1 -b${ARDUINOISP_SERIALSPEED} -Uflash:w:$2.hex:i -Ueeprom:w:$2.eep:i
}

tn84upload()
{
  avrdude -v -pattiny84 -cstk500v1 -P/dev/tty$1 -b${ARDUINOISP_SERIALSPEED} -Uflash:w:$2.hex:i -Ueeprom:w:$2.eep:i
}

m328upload()
{
  avrdude -v -pm328p -cstk500v1 -P/dev/tty$1 -b${ARDUINOISP_SERIALSPEED} -Uflash:w:$2.hex:i -Ueeprom:w:$2.eep:i
}

nanoupload()
{
  avrdude -v -p atmega328p -c arduino -P /dev/tty$1 -b ${ARDUINONANO_SERIALSPEED} -D -U flash:w:$2.hex:i -U eeprom:w:$2.eep:i
}

tn85i8fuse()
{
  avrdude -v -pattiny85 -cstk500v1 -P/dev/tty$1 -b${ARDUINOISP_SERIALSPEED} -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
}

tn84i8fuse()
{
  avrdude -v -pattiny84 -cstk500v1 -P/dev/tty$1 -b${ARDUINOISP_SERIALSPEED} -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
}

m328i8bootloader()
{
  avrdude -v -pm328p -cstk500v1 -P/dev/tty$1 -b${ARDUINOISP_SERIALSPEED} -Uflash:w:ATmegaBOOT_168_atmega328_pro_8MHz.hex:i -Ulock:w:0xCF:m
}

m328i8fuse()
{
  avrdude -v -pm328p -cstk500v1 -P/dev/tty$1 -b${ARDUINOISP_SERIALSPEED} -Ulfuse:w:0xe2:m -Uhfuse:w:0xd9:m -Uefuse:w:0xff:m
}

m328e16bootloader() 
{
  # 'normal' configuration, aka just like a regular Uno @16Mhz
  avrdude -v -pm328p -cstk500v1 -P/dev/tty$1 -b${ARDUINOISP_SERIALSPEED} -U lock:w:0x3F:m -U lfuse:w:0xFF:m -U hfuse:w:0xDE:m -U efuse:w:0x05:m -U flash:w:${ATMEGA328_BOOT_IMG} -U lock:w:0x0F:m
}

m328e16fuse()
{
  avrdude -v -pm328p -cstk500v1 -P/dev/tty$1 -b${ARDUINOISP_SERIALSPEED} -e -Ulock:w:0x3F:m -Uefuse:w:0x05:m -Uhfuse:w:0xDA:m -Ulfuse:w:0xFF:m
}

