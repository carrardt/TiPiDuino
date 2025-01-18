#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <HWSerialIO.h>
#include <InputStream.h>
#include <math.h>

using namespace avrtl;

void setup()
{
	cli();
	TCCR0B = (TCCR0B & 0b11111000) | 0b010;
	
	DDRD |= 0xFC; // 6 highest bits bits from port D (pins 2,3,4,5,6,7)
	DDRB |= 0X03; // 2 lowest bits from port B (pins 8,9)
}

static inline void writeLeds( uint8_t x )
{
	uint8_t d = PIND;
	uint8_t b = PINB;
	uint8_t tb = (b&0xFC) | ((x>>6)&0x03);
	uint8_t td = (d&0X03) | ((x<<2)&0xFC);
	PIND = d^td;
	PINB = b^tb;
}

void loop()
{
	for(int i=0;i<256;i++)
	{
		writeLeds(i); DelayMicrosecondsFast(500000);
	}
}
