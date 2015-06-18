#include <AvrTLPin.h>
#include <AvrTLSignal.h>
//#include <HWSerialIO.h>

using namespace avrtl;

#define LASER_PIN 12

auto laser = StaticPin<LASER_PIN>();

void setup()
{
	laser.SetOutput();
}

void loop()
{
	laser = true;
	DelayMicroseconds( 1000 );
	laser = false;
	DelayMicroseconds( 3000 );
}
