# ARduino model
MCUMODEL=atmega328p

# avr tools root dir
AVRDIR="$FAKECHROOT_BASE/usr/local/avr/bin"
AVRX="env LD_PRELOAD="

# Arduino library dirs
export ARDUINOCORE="$FAKECHROOT_BASE/usr/local/arduino_core"
export ARDUINOLIBS="$FAKECHROOT_BASE/usr/local/arduino_libs"

# compiler flags
ARCH_FLAGS="-mmcu=$MCUMODEL -DF_CPU=16000000L -DUSB_VID=null -DUSB_PID=null -DARDUINO=105"
FLAGS="-O2 -Os -w $ARCH_FLAGS -DIDE=\"TiDuino\""
AVR_C_FLAGS="$FLAGS"
AVR_CXX_FLAGS="-std=c++11 $FLAGS"


export AVR_CC="$AVRX $AVRDIR/avr-gcc $AVR_C_FLAGS" 
export AVR_CXX="$AVRX $AVRDIR/avr-g++ $AVR_CXX_FLAGS"
export AVR_AR="$AVRX $AVRDIR/avr-ar"
export AVR_STRIP="$AVRX $AVRDIR/avr-strip"
export AVR_LD="$AVRX $AVRDIR/avr-gcc -Os -Wl,--gc-sections -mmcu=atmega328p" # -o output.elf obj1.o .. objN.o
export AVR_MKHEX="$AVRX $AVRDIR/avr-objcopy -O ihex -R .eeprom" # input.elf output.hex
export AVR_DUDE="su -c $AVRDIR/avrdude -C $AVRDIR/avrdude.conf" 

