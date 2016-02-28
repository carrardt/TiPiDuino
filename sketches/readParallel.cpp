#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <HWSerialIO.h>
#include <InputStream.h>
#include <math.h>
#include <HWSerialIO.h>
#include <PrintStream.h>

//#define ONLY_5BITS 1

#define PWM_PIN 13

using namespace avrtl;

HWSerialIO serialIO;
PrintStream serialOut;

// Servo 1 : 90-960 ==> 500+90*2=680 TO 500+960*2=2420. safe = [700;2400] uSec, input value = [100;950];

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

static uint8_t par10_lockBit = 0;
static uint16_t par10_value = 0;

static void par10_read()
{
	uint8_t r1=0,r2=0;
	r1 = PINB;
	r2 = PIND;
	par10_lockBit = ( (r1&0x10) == 0 ) ? 0 : 1;
	par10_value = ( ((uint16_t)(r1&0x0F))<<6 ) | ( r2>>2 );
#ifdef ONLY_5BITS
	par10_value &= 0x03E0;
#endif
}

void setup()
{
	par10_init();

	serialIO.begin(9600);
	serialOut.begin( &serialIO );
}

void loop()
{
	par10_read();
	serialOut << "value="<<par10_value<<", lock="<<par10_lockBit<<endl;
	DelayMicroseconds(500000);
}
