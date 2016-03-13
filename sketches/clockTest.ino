#include <AvrTLPin.h>
#include <AvrTLSignal.h>

using namespace avrtl;

#define PWM_PIN 3

auto pwm = StaticPin<PWM_PIN>();
void setup()
{
	pwm.SetOutput();
	cli();
}

void loop()
{
	static uint16_t counter = 0;
	static uint8_t t = 0;
	while( TCNT0 == t );
	t=TCNT0;
	counter = counter+1;
	pwm.Set( (counter>>12)&1 );
}
