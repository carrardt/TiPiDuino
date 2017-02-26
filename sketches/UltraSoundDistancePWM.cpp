#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <TimeScheduler.h>

using namespace avrtl;

#define TRIGGER_PIN 3
#define ECHO_PIN 2
#define PWM_PIN 13

#define SMOOTHING 2 // 0,1,2 or 3 level of input value smoothing

static auto trigger = StaticPin<TRIGGER_PIN>();
static auto echo = StaticPin<ECHO_PIN>();
static auto pwm = StaticPin<PWM_PIN>();

static TimeScheduler ts;

void setup()
{
	pwm.SetOutput();
	trigger.SetOutput();
	echo.SetInput(); 
}

void loop()
{
	uint16_t echo_length = 0;
	uint16_t echo_gap = 0;
	uint16_t value = 1024;
	uint16_t targetValue = 1024;
	uint8_t counter = 0;
	
	ts.start();

	while( true )
	{
		ts.exec( 600,
				[&echo_gap, &echo_length]()
				{ 
					pwm = HIGH;
					echo_gap = 0;
					echo_length = 0;
				} );

		ts.loop( 3400, [value](int16_t t){ pwm = (t<value); } );

		ts.exec( 40, [counter]() { pwm = LOW; } ); // just in case

		ts.exec( 10, [counter]() { trigger = (counter==0); } );

		ts.exec( 50, []() {	trigger = LOW; } );

		ts.loop( 8000, // we have 5 milliseconds to listen for an echo
				[&echo_gap,&echo_length](int16_t t)
				{
					if( echo.Get() )
					{
						if( echo_gap==0 ) echo_gap = t;
					}
					else
					{
						if( echo_gap!=0 && echo_length==0 )
						{
							echo_length = t - echo_gap;
						}
					}
				} );

		ts.exec( 7900,
				[&value,&targetValue,&counter,echo_gap,echo_length]()
				{
					if( counter==0 && echo_gap>512 && echo_gap<1536 && echo_length>384 && echo_length<1536)
					{
						targetValue = 768+echo_length;
						if( targetValue < 768 ) targetValue = 768;
						if( targetValue > 2560 ) targetValue = 2560;
						targetValue -= 600;
					}
					value = (targetValue+value)>>1;
					++ counter;
					if( counter == 3 ) { counter = 0; }
				} );
	}

	ts.stop();
}

