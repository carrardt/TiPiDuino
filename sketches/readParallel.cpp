#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <HWSerialIO.h>
#include <PrintStream.h>

using namespace avrtl;

#define LATENCY_BENCHMARK 1

HWSerialIO serialIO;
PrintStream serialOut;

/*
 * Atmega 328:
 * pins 0-7 on port D. we use only 6 bits (pins 2-7) from port D. pins 0-1 are used for serial
 * pins 0-7 of port D are mapped to bits 0-7 for register B
 * pins 8-13 on port B. mapped to bits 0-5 of register B. we use 4 bits (0-3) from port B.
 * pin 12 (bit 4 of port B) is used as a lock bit for data integrity
 * 
*/
// uses pins 2-11 for 10 bits input, pin 12 for lock bit.
// data can be read only when lock bit is 0
static void par10_init()
{
	DDRD &= 0x03; // 6 highest bits bits from port D 
	DDRB &= 0XE0; // 4 lowest bits from port B, plus 5th bit as a lock bit (pin 12)
}

static uint16_t par10_read()
{
	uint8_t r1=0,r2=0;
	do
	{
		r1 = PINB;
		r2 = PIND;
	} while( r1&0x10 );// wait for lock bit t be clear
	return ( ((uint16_t)(r1&0x0F))<<6 ) | ( r2>>2 );
}


void setup()
{
	serialIO.begin(9600);
	serialOut.begin( &serialIO );
	par10_init();
	serialOut<<"ready"<<endl;
}

#ifdef LATENCY_BENCHMARK

void loop()
{
	uint8_t old_SREG = SREG;
	cli();

	uint16_t v = 0;
	uint16_t missed = 0;
	v = par10_read();
	while( v != 1023 ) v = par10_read();

	for(uint16_t i=0;i<32768;i++)
	{
		uint16_t v2 = par10_read();
		while( v2 == v ) v2 = par10_read();
		if( ((v+1)&1023) != v2 ) ++missed;
		v=v2;
	}
	SREG = old_SREG;
	serialOut<<"missed: "<<missed<<endl;
	// serialOut.print(value,16,4);
}

#else

static uint16_t lastValue = 0;

void loop()
{
	uint16_t value = par10_read();
	if(value==lastValue) return;
	lastValue = value;
	serialOut<<value<<endl;
}

#endif	
