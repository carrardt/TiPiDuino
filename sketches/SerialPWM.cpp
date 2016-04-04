#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <TimeScheduler.h>

using namespace avrtl;

#define SERIAL_RX_PIN 2
#define SERIAL_TX_PIN 3
#define PWM_PIN 13

#define SMOOTHING 2 // 0,1,2 or 3 level of input value smoothing

static auto pwm = StaticPin<PWM_PIN>();
static auto rx = StaticPin<2>();
static auto tx = StaticPin<3>();
static auto serialIO = make_softserial<57600>(rx,tx);

static TimeScheduler ts;

void setup()
{
	serialIO.begin();
	pwm.SetOutput();
}

void loop()
{
	uint16_t value = 1024;
	uint16_t targetValue = 1024;
	
	ts.start();

	while( true )
	{
		ts.exec( 400, []() { pwm = HIGH; } );
		ts.loop( 2000, [value](int16_t t){ pwm = (t<value); } );
		ts.exec( 100, [counter]() { pwm = LOW; } ); // just in case
		ts.loop( 5000, // we have 5 milliseconds to listen for serial commands
				[](int16_t t)
				{
					uint8_t x = rx.Get();
				} );

		ts.exec( 2500,
				[]()
				{
					value = ( (value<<SMOOTHING)-value+targetValue ) >> SMOOTHING ;
				} );
	}

	ts.stop();
}

