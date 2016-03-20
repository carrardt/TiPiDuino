#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTLSignal.h>

using namespace avrtl;

#define TRIGGER_PIN 3
#define ECHO_PIN 2
#define PWM_PIN 13

#define SMOOTHING 1 // 0,1,2 or 3 level of input value smoothing

auto trigger = StaticPin<TRIGGER_PIN>();
auto echo = StaticPin<ECHO_PIN>();
auto pwm = StaticPin<PWM_PIN>();

void setup()
{
	pwm.SetOutput();
	trigger.SetOutput();
	echo.SetInput(); 
}

void loop()
{
	constexpr uint16_t CycleTicks =  microsecondsToTicks(10050);
	constexpr uint16_t TriggerTicks =  microsecondsToTicks(10);
	constexpr uint16_t EchoTimeOutTicks =  microsecondsToTicks(2500);
	constexpr uint16_t EchoGapMinTicks =  microsecondsToTicks(1000);
	constexpr uint16_t EchoLengthMinTicks =  microsecondsToTicks(300);
	constexpr uint16_t EchoLengthMaxTicks =  microsecondsToTicks(2200);
	
	uint16_t echo_length = 0;
	uint16_t echo_gap = 0;
	uint16_t value = microsecondsToTicks(768);
	uint16_t targetvalue = microsecondsToTicks(768);
	uint8_t counter = 0;
	
	uint16_t HighPeriodTicks = value;
	uint16_t LowPeriodTicks = 0;
	uint16_t nextSwitchTime = 0;		
	uint16_t wallClock = 0;
	uint16_t tmpClock = 0;

	SCOPED_SIGNAL_PROCESSING;
		
	INIT_CLOCK_COUNTER16();		
	pwm.Set(true);
	while( HighPeriodTicks < CycleTicks )
	{
		// shift timer back so that 16bits wallclock is always enough
		REWIND_CLOCK_COUNTER(LowPeriodTicks);
		
		do {
			UPDATE_CLOCK_COUNTER();
			wallClock = CLOCK_ELAPSED();
		} while( wallClock < HighPeriodTicks );

		// as soon as we detect high time is elapsed, set to low
		pwm.Set(false);
		
		// keep timer in sync
		UPDATE_CLOCK_COUNTER();
		
		// shift timer back so that 16bits wallclock is always enough
		REWIND_CLOCK_COUNTER(HighPeriodTicks);
		
		// update timings
		LowPeriodTicks = CycleTicks - HighPeriodTicks;

		/**** BEGIN Ultra-Sound MEASURE *****/
		UPDATE_CLOCK_COUNTER();
		if( counter == 6 )
		{
			trigger = true;
			UPDATE_CLOCK_COUNTER();
			tmpClock = CLOCK_ELAPSED();
			while( (CLOCK_ELAPSED()-tmpClock) < TriggerTicks ) { UPDATE_CLOCK_COUNTER(); }
			trigger = false;
			/*tmpClock = CLOCK_ELAPSED();
			while( !echo.Get() && (CLOCK_ELAPSED()-tmpClock) < EchoTimeOutTicks ) { UPDATE_CLOCK_COUNTER(); }
			echo_gap = CLOCK_ELAPSED() - tmpClock;
			while( echo.Get() && (CLOCK_ELAPSED()-tmpClock) < (2*EchoTimeOutTicks) ) { UPDATE_CLOCK_COUNTER(); }
			echo_length = CLOCK_ELAPSED() - tmpClock - echo_gap;
			if( echo_gap>EchoGapMinTicks && echo_length>EchoLengthMinTicks && echo_length<EchoLengthMaxTicks )
			{
				targetvalue = echo_length;
			}*/
			counter = 0;
		}
		else { ++ counter; }
		//UPDATE_CLOCK_COUNTER();
		//value = ( (value<<SMOOTHING) - value + targetvalue ) >> SMOOTHING;
		/**** END US MEASURE *****/
		
		UPDATE_CLOCK_COUNTER();
		HighPeriodTicks = value;
		bool highState = (HighPeriodTicks>0);
		
		do {
			UPDATE_CLOCK_COUNTER();
			wallClock = CLOCK_ELAPSED();
		} while( wallClock<LowPeriodTicks );

		// as soon as we detect low time is elapsed, set to high
		pwm.Set(highState);
	}
}

