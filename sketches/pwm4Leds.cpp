#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <HWSerialIO.h>
#include <InputStream.h>
#include <math.h>

using namespace avrtl;

auto led1 = StaticPin<2>();
auto led2 = StaticPin<3>();
auto led3 = StaticPin<4>();
auto led4 = StaticPin<5>();

void setup()
{
	led1.SetOutput();
	led2.SetOutput();
	led3.SetOutput();
	led4.SetOutput();
}

void loop()
{
	SCOPED_SIGNAL_PROCESSING;
	pulsePWMFastTicks<1000,200>( led1, 500 );
	pulsePWMFastTicks<1000,200>( led2, 500 );
	pulsePWMFastTicks<1000,200>( led3, 500 );
	pulsePWMFastTicks<1000,200>( led4, 500 );
}
