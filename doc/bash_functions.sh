# in case you want to manually upload a .hex to an avr, using an ArduinoAsISP, you can use these 
# shell functions :

export ARDUINOISP_SERIALSPEED=19200 # standard value as in the ArduinoISP original sketch

tn85upload()
{
        avrdude -v -pattiny85 -cstk500v1 -P/dev/tty$1 -b${ARDUINOISP_SERIALSPEED} -Uflash:w:$2:i
}

tn84upload()
{
        avrdude -v -pattiny84 -cstk500v1 -P/dev/tty$1 -b${ARDUINOISP_SERIALSPEED} -Uflash:w:$2:i
}


tn85i8fuse()
{
	avrdude -v -pattiny85 -cstk500v1 -P/dev/tty$1 -b${ARDUINOISP_SERIALSPEED} -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
}

tn84i8fuse()
{
	avrdude -v -pattiny84 -cstk500v1 -P/dev/tty$1 -b${ARDUINOISP_SERIALSPEED} -U lfuse:w:0xe2:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m
}

m328upload()
{
        avrdude -v -pm328p -cstk500v1 -P/dev/tty$1 -b${ARDUINOISP_SERIALSPEED} -Uflash:w:$2:i
}

m328i8fuse()
{
        avrdude -v -pm328p -cstk500v1 -P/dev/tty$1 -b${ARDUINOISP_SERIALSPEED} -Ulfuse:w:0xe2:m -Uhfuse:w:0xd9:m -Uefuse:w:0xff:m
}

m328e16fuse()
{
        avrdude -v -pm328p -cstk500v1 -P/dev/tty$1 -b${ARDUINOISP_SERIALSPEED} -e -Ulock:w:0x3F:m -Uefuse:w:0x05:m -Uhfuse:w:0xDA:m -Ulfuse:w:0xFF:m
}


