#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <HWSerialIO.h>
#include <InputStream.h>
#include <math.h>

using namespace avrtl;

#define PWM_PIN 12
auto pwm = StaticPin<PWM_PIN>();
constexpr uint16_t cycleTicks =  microsecondsToTicks(10000);

// uses pins 2-11 for 10 bits input
static void par10_init()
{
	DDRD &= 0x03;
	DDRB &= 0XF0;
}

static uint16_t par10_read()
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

void setup()
{
	par10_init();
	pwm.SetOutput();
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
