#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTLSignal.h>

using namespace avrtl;

#define TRIGGER_PIN 3
#define ECHO_PIN 2
#define PWM_PIN 13

#define SMOOTHING 1 // 0,1,2 or 3 level of input value smoothing

static auto trigger = StaticPin<TRIGGER_PIN>();
static auto echo = StaticPin<ECHO_PIN>();
static auto pwm = StaticPin<PWM_PIN>();

/*
#define INIT_CLOCK_COUNTER() 	uint8_t _t=TCNT0,_t0=_t; int32_t _m=-_t0
#define INIT_CLOCK_COUNTER16() 	uint8_t _t=TCNT0,_t0=_t; int16_t _m=-_t0
#define REWIND_CLOCK_COUNTER(x) do{ _m -= (x); }while(0)
#define RESET_CLOCK_COUNTER() do{ _m=0; _t=TCNT0; }while(0)
#define UPDATE_CLOCK_COUNTER() 	do{ uint8_t _t2=TCNT0; if(_t2<_t)_m+=256; _t=_t2; }while(0)
#define CLOCK_ELAPSED() 		(_m+_t)
*/

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
		ts.exec( value,
				[&echo_gap, &echo_length]()
				{ 
					pwm=HIGH;
					echo_gap = 0;
					echo_length = 0;
				} );
				
		ts.exec( 40 ,
				[counter]()
				{
					pwm=LOW;
				} );

		ts.exec( 10 ,
				[counter]()
				{
					trigger = (counter==0);
				} );
				
		ts.exec( 50,
				[]()
				{
					trigger=LOW;
				} );
		
		ts.loop( 5000,
				[&echo_gap,&echo_length](int16_t t) {
					bool e = echo.Get();
					if( echo_gap==0 && e ) { echo_gap = t; }
					if( echo_gap!=0 && !e ) { echo_length = t - echo_gap; }
				} );

		ts.exec( 10000-(value+5100),
				[&value,&targetValue,&counter,echo_gap,echo_length](){
					if( echo_gap>1024 && echo_length>320 )
					{
						targetValue = microsecondsToTicks( echo_length/2 + 350 );
					}
					value = ( (value<<SMOOTHING)-value+targetValue ) >> SMOOTHING ;
					++ counter;
					if( counter == 6 ) counter = 0;
				} );
		
	}
}

