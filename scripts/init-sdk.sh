# current dir
DIR=`pwd`

# Arduino library dirs
export ARDUINOCORE=$DIR/arduino_core
export ARDUINOLIBS=$DIR/arduino_libs

# dev sources
export ETL_DIR=$DIR/dev/ETL-master

# compiler flags
ARCH_FLAGS="-mmcu=atmega328p -DF_CPU=16000000L -DUSB_VID=null -DUSB_PID=null -DARDUINO=105"
FLAGS="-O2 -Os -w $ARCH_FLAGS -DIDE=\"TiDuino\""
AVR_C_FLAGS="$FLAGS"
AVR_CXX_FLAGS="-std=c++11 $FLAGS"

# avr tools root dir
AVRROOT="$DIR/avr"
AVRGCCDIR="$DIR/avr/bin"

export AVR_GCC="$AVRGCCDIR/avr-gcc $AVR_C_FLAGS"
export AVR_GXX="$AVRGCCDIR/avr-g++ $AVR_CXX_FLAGS"
export AVR_AR="$AVRGCCDIR/avr-ar"
export AVR_STRIP="$AVRGCCDIR/avr-strip"
export AVR_LD="$AVRGCCDIR/avr-gcc -Os -Wl,--gc-sections -mmcu=atmega328p" # -o output.elf obj1.o .. objN.o
export AVR_MKHEX="$AVRGCCDIR/avr-objcopy -O ihex -R .eeprom" # input.elf output.hex

export AVRDUDE="$AVRROOT/bin/avrdude -C $AVRROT/etc/avrdude.conf"
alias inocc='$DIR/inocc'
alias flash='$DIR/upload'

