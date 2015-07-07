#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <HWSerialIO.h>
#include <PrintStream.h>

using namespace avrtl;

// #define LATENCY_BENCHMARK 1

HWSerialIO serialIO;
PrintStream serialOut;

void par10_init()
{
	DDRD &= 0x03;
	DDRB &= 0XF0;
}

uint16_t par10_read()
{
	uint16_t v1 = (PIND>>2) | ( ((uint16_t)(PINB&0x0F)) << 6 );
	uint16_t v2 = (PIND>>2) | ( ((uint16_t)(PINB&0x0F)) << 6 );
	while( v2 != v1 )
	{
		v1 = v2;
		v2 = (PIND>>2) | ( ((uint16_t)(PINB&0x0F)) << 6 );
	}
	return v2;
}

static uint8_t old_SREG;

void setup()
{
	serialIO.begin(9600);
	serialOut.begin( &serialIO );
	par10_init();
	serialOut<<"ready"<<endl;
#ifdef LATENCY_BENCHMARK
	old_SREG = SREG;
	cli();
	while( par10_read()!=1 );
#endif
}

static uint16_t lastValue = 0;
static uint16_t missed = 0;
static uint16_t missedValues[128];

void loop()
{
	uint16_t value = par10_read();
	if(value==lastValue) return;
	
#ifdef LATENCY_BENCHMARK
	if( (lastValue+1) != value )
	{
		missedValues[missed++] = lastValue+1;
	}
	if( value==0 )
	{
		SREG = old_SREG;
		serialOut<<"missed: "<<missed<<endl;
		for(int i=0;i<missed;i++)
		{
			serialOut<<missedValues[i]<<endl;
		}
		// serialOut.print(value,16,4);
		old_SREG = SREG;
		cli();
	}
#endif

	lastValue = value;

#ifndef LATENCY_BENCHMARK
	serialOut<<value<<endl;
#endif	

}
