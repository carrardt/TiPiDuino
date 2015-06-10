#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <HWSerialIO.h>
#include <PrintStream.h>

using namespace avrtl;

HWSerialIO serialIO;
PrintStream serialOut;

void setup()
{
	serialIO.begin(9600);
	serialOut.begin( &serialIO );
	DDRD &= 0x03;
	DDRB &= 0XF0;
}

void loop()
{
	uint16_t value = ( (PIND>>2) << 4 ) | ( PINB & 0x0F );
	serialOut.print(value,16,4);
	serialOut << endl;
	DelayMicroseconds(1000000);
}
