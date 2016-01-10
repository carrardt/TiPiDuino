#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <HWSerialIO.h>
#include <InputStream.h>
#include <math.h>
#include <HWSerialIO.h>
#include <PrintStream.h>

using namespace avrtl;


HWSerialIO serialIO;
PrintStream serialOut;

#define PWM_PIN 13
auto pwm = StaticPin<PWM_PIN>();
constexpr uint16_t cycleTicks =  microsecondsToTicks(10000);

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

static int16_t par10_lastValue = 512;
static int16_t par10_targetValue = 512;
static int16_t par10_speed = 0;
static uint16_t par10_counter = 0;

static uint16_t par10_read()
{
	uint8_t r1=0,r2=0;
	r1 = PINB;
	r2 = PIND;
	if( (r1&0x10) == 0 ) // input is valid only if lock bit is clear (0)
	{
		par10_targetValue = ( ((uint16_t)(r1&0x0F))<<6 ) | ( r2>>2 );
	}
	par10_lastValue = (par10_lastValue*3+par10_targetValue)/4;
	return par10_lastValue;
}

static uint8_t old_SREG;

void setup()
{
	par10_init();
	pwm.SetOutput();

	serialIO.begin(9600);
	serialOut.begin( &serialIO );
	serialOut<<"Parallel to PWM ready"<<endl;
	
	old_SREG = SREG;
	cli();
}

static uint16_t getPWMTicks()
{
	uint16_t value = 500 + par10_read()*2;
	return microsecondsToTicks(value);
}

void loop()
{
	loopPWM<cycleTicks>( pwm, getPWMTicks );
}
