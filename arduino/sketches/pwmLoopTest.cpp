#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <HWSerialIO.h>
#include <InputStream.h>
#include <math.h>

using namespace avrtl;

#define PWM_PIN 12

auto pwm = StaticPin<PWM_PIN>();
constexpr uint16_t cycleTicks =  microsecondsToTicks(10000);

void setup()
{
	pwm.SetOutput();
}

static uint16_t getPWMTicks()
{
	static uint16_t value = 1000;
	static int8_t inc = 1;
	value += inc;
	if( value >= 2200 ) inc = -1;
	if( value <= 800 ) inc = 1;
	return microsecondsToTicks(value);
}

void loop()
{
	loopPWM<cycleTicks>( pwm, getPWMTicks );
}
