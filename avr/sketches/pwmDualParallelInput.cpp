#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <HWSerialIO.h>
#include <InputStream.h>
#include <math.h>
#include <HWSerialIO.h>
#include <PrintStream.h>

//#define ONLY_5BITS 1

#define PWM_PIN 13
#define SMOOTHING 1 // 0,1,2 or 3 level of input value smoothing

using namespace avrtl;

HWSerialIO serialIO;
PrintStream serialOut;

auto dualpwm = DualPin<12,13>();
constexpr uint16_t cycleTicks =  19809;// this is valid only for 16Mhz clock, otherwise use this => microsecondsToTicks(9904);

// Servo 1 : 90-960 ==> 500+90*2=680 TO 500+960*2=2420. safe = [700;2400] uSec, input value = [100;950];

/*
 * Atmega 328:
 * pins 0-7 on port D. we use only 6 bits (pins 2-7) from port D. pins 0-1 are used for serial
 * pins 0-7 of port D are mapped to bits 0-7 for register B
 * pins 8-13 on port B. mapped to bits 0-5 of register B. we use 4 bits (0-3) from port B.
 * pin 12 (bit 4 of port B) is used as a lock bit for data integrity
 * 
*/
// uses pins 2-11 for 10 bits input (2x 5bits)
static void par10_init()
{
	DDRD &= 0x03; // 6 highest bits bits from port D 
	DDRB &= 0XF0; // 4 lowest bits from port B
}

static int16_t Value1 = 1;
static int16_t Value2 = 1;

static void par10_read()
{
	uint8_t r1=0,r2=0;
	r1 = PINB;
	r2 = PIND;
	int16_t X = ( ((uint16_t)(r1&0x0F))<<6 ) | ( r2>>2 );
	Value1 = X >> 5;
	Value2 = X & 0x1F;
}

static uint8_t old_SREG;

void setup()
{
	par10_init();
	dualpwm.SetOutput();

	serialIO.begin(9600);
	serialOut.begin( &serialIO );
	serialOut<<"Parallel to Dual PWM ready"<<endl;
	serialOut<<"Cycle Ticks = "<<cycleTicks<<endl;
	
	old_SREG = SREG;
	cli();
}

static uint16_t getPWMTicks1()
{
	par10_read();
	uint16_t value = 512 + (Value1<<6);
	return microsecondsToTicks(value);
}
static uint16_t getPWMTicks2()
{
	uint16_t value = 512 + (Value2<<6);
	return microsecondsToTicks(value);
}

void loop()
{
	loopDualPWM<cycleTicks>( dualpwm, getPWMTicks1, getPWMTicks2 );
}
