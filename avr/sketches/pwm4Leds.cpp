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
/*
auto led5 = StaticPin<6>();
auto led6 = StaticPin<7>();
auto led7 = StaticPin<8>();
auto led8 = StaticPin<9>();
*/

void setup()
{
	cli();
	TCCR0B = (TCCR0B & 0b11111000) | 0b010;
	led1.SetOutput();
	led2.SetOutput();
	led3.SetOutput();
	led4.SetOutput();
/*
	led5.SetOutput();
	led6.SetOutput();
	led7.SetOutput();
	led8.SetOutput();
*/
}

void loop()
{
	//SCOPED_SIGNAL_PROCESSING;
	pulsePWMFastTicks<1000,200>( led1, 500 );
	pulsePWMFastTicks<1000,200>( led2, 500 );
	pulsePWMFastTicks<1000,200>( led3, 500 );
	pulsePWMFastTicks<1000,200>( led4, 500 );
/*
	pulsePWMFastTicks<1000,200>( led5, 500 );
	pulsePWMFastTicks<1000,200>( led6, 500 );
	pulsePWMFastTicks<1000,200>( led7, 500 );
	pulsePWMFastTicks<1000,200>( led8, 500 );
*/
}
