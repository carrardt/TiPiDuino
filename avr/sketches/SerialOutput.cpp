#include <AvrTL.h>
#include <AvrTLPin.h>

#include "BasicIO/PrintStream.h"
#include "BasicIO/InputStream.h"

#include <avr/interrupt.h>

#define SERIAL_SPEED 19200

#include "HWSerialNoInt/HWSerialNoInt.h"
ByteStreamAdapter<HWSerialNoInt,10000UL> serialIO;
PrintStream cout;
int n = 0;

void setup()
{
	cli();
	serialIO.m_rawIO.begin(SERIAL_SPEED);
	cout.begin( &serialIO );
}


void loop()
{
	n = ( n + 1 ) % 1000;
	cout << n << '\n';
}

