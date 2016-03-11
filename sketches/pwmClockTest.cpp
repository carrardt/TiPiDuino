#include <AvrTLPin.h>
#include <AvrTLSignal.h>

using namespace avrtl;

#define PWM_PIN 3

auto pwm = StaticPin<PWM_PIN>();
constexpr uint16_t cycleTicks = microsecondsToTicks(10050);

void setup()
{
	pwm.SetOutput();
}

static uint16_t value = microsecondsToTicks(2000);

static uint16_t getPWMTicks()
{
	return value;
}

void loop()
{
	loopPWM<cycleTicks>( pwm, getPWMTicks );
}
