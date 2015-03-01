VARIANT=standard
#VARIANT=micro
#VARIANT=leonardo
#VARIANT=mega

IGNORE_LIBRARIES="Esplora WiFi"

CP="busybox cp"
RM="busybox rm"
MV="busybox mv"
MKDIR="busybox mkdir"
FIND="busybox find"
DIRNAME="busybox dirname"
BASENAME="busybox basename"
XARGS="busybox xargs"
GREP="busybox grep"
WC="busybox wc"

BASEDIR=`$DIRNAME $0`
if [ "x$BASEDIR" = "x." ]
then
	BASEDIR=`pwd`
fi
ARDUINO_LIBRARIES=$BASEDIR/libraries
ARDUINO_CORE=$BASEDIR/arduino/cores/arduino
ARDUINO_VARIANT=$BASEDIR/arduino/variants/$VARIANT
ADDONLIB_DIR=$BASEDIR/arduino_libs
CORELIB_DIR=$BASEDIR/arduino_core

echo "BASEDIR=$BASEDIR"

source $BASEDIR/init-sdk.sh
CC="$AVR_GCC"
CXX="$AVR_GXX"
AR="$AVR_AR"

echo "* Core sources: $ARDUINO_CORE"
echo "* Libraries sources: $ARDUINO_LIBRARIES"
echo "* Core build: $CORELIB_DIR"
echo "* Libraries build: $ADDONLIB_DIR"
echo "* CC: $CC"
echo "* CXX: $CXX"
echo "* AR: $AR"

echo "Delete previous version ..."
$RM -fr $ADDONLIB_DIR $CORELIB_DIR
$MKDIR  $ADDONLIB_DIR $CORELIB_DIR

echo "Compile Arduino core library ..."
$FIND $ARDUINO_CORE -type f -exec cp {} $CORELIB_DIR/ \;
$FIND $ARDUINO_VARIANT -type f -exec cp {} $CORELIB_DIR/ \;
cd $CORELIB_DIR
$CC -c -I. *.c
$CXX -c -I. *.cpp
$AR rcs libcore.a *.o
$RM -f *.o *.cpp *.c
cd ..

echo "Copy additional libraries header files ..."
$MKDIR $ADDONLIB_DIR/utility
for ALIB in `ls $ARDUINO_LIBRARIES`
do
	ADIR=$ARDUINO_LIBRARIES/$ALIB
	if [ -d $ADIR ]
	then
		echo "    $ALIB"
		if [ -d $ADIR/utility ]
		then
			$FIND $ADIR/utility -name "*.h" -exec $CP {} $ADDONLIB_DIR/utility \;
		fi
		$CP $ADIR/$ALIB.h $ADDONLIB_DIR/
	fi
done

echo "Compile additional libraries ..."
for ALIB in `ls $ARDUINO_LIBRARIES`
do
	SKEEP_LIB=`echo $IGNORE_LIBRARIES | $GREP $ALIB | $WC -l`
	ADIR=$ARDUINO_LIBRARIES/$ALIB
	if [ -d $ADIR ]
	then
	    if [ $SKEEP_LIB -eq 0 ]
	    then
		echo "    $ALIB"
		ABUILD=$ADDONLIB_DIR/$ALIB
		$MKDIR $ABUILD
		AFLAGS="-I$ADIR -I$ADDONLIB_DIR -I$CORELIB_DIR"
		if [ -d $ADIR/utility ]
		then
			AFLAGS="$AFLAGS -I$ADIR/utility"
			cd $ABUILD
			$FIND $ADIR/utility -name "*.c" -exec $CC $AFLAGS -c {} \;
			$FIND $ADIR/utility -name "*.cpp" -exec $CXX $AFLAGS -c {} \;
			cd - 2>&1 >/dev/null
		fi
		$CXX $AFLAGS -c $ADIR/$ALIB.cpp -o $ABUILD/$ALIB.o
		$AR rcs $ADDONLIB_DIR/lib$ALIB.a $ABUILD/*.o
		$RM -fr $ABUILD
	    fi
	fi
done

echo "Finished"

