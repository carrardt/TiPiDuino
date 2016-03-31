#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTLSignal.h>

using namespace avrtl;

#define TRIGGER_PIN 3
#define ECHO_PIN 2
#define PWM_PIN 13

#define SMOOTHING 2 // 0,1,2 or 3 level of input value smoothing

static auto trigger = StaticPin<TRIGGER_PIN>();
static auto echo = StaticPin<ECHO_PIN>();
static auto pwm = StaticPin<PWM_PIN>();

struct TimeScheduler
{
	int16_t m_wallclock;
	uint8_t m_t;
	
	inline void start()
	{
		m_wallclock = 0;
		m_t = TCNT0;
	}
		
	inline int16_t wallclock()
	{
		uint8_t t2 = TCNT0;
		m_wallclock += (uint8_t)( t2 - m_t );
		m_t = t2;
		return m_wallclock;
	}
	
	template<typename FuncT>
	inline void exec( int16_t t, FuncT f )
	{
		t = avrtl::microsecondsToTicks(t);
		f();
		while( wallclock() < t );
		m_wallclock -= t;				
	}

	template<typename FuncT>
	inline void loop( int16_t t, FuncT f )
	{
		t = avrtl::microsecondsToTicks(t);
		int16_t w;
		while( (w=wallclock()) < t ) { f( avrtl::ticksToMicroseconds(w) ); }
		m_wallclock -= t;				
	}
	
};

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
	
	SCOPED_SIGNAL_PROCESSING;
	ts.start();

	while( true )
	{
		ts.exec( 400,
				[&echo_gap, &echo_length]()
				{ 
					pwm=HIGH;
					echo_gap = 0;
					echo_length = 0;
				} );

		ts.loop( 2000, [value](int16_t t){ pwm = (t<value); } );

		ts.exec( 40, [counter]() { pwm = LOW; } ); // just in case

		ts.exec( 10, [counter]() { trigger = (counter==0); } );

		ts.exec( 50, []() {	trigger = LOW; } );

		ts.loop( 5000, // we have 5 milliseconds to listen for an echo
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

		ts.exec( 2500,
				[&value,&targetValue,&counter,echo_gap,echo_length]()
				{
					if( counter==0 && echo_gap>512 && echo_length>256)
					{
						targetValue = 256 + echo_length;
						if( targetValue < 500 ) targetValue = 500;
						if( targetValue > 2400 ) targetValue = 2400;
						targetValue -= 400;
					}
					value = ( (value<<SMOOTHING)-value+targetValue ) >> SMOOTHING ;
					++ counter;
					if( counter == 6 ) counter = 0;
				} );
		
	}
}

