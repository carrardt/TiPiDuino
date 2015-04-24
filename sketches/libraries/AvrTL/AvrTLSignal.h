#ifndef __AVRTL_SIGNAL_H
#define __AVRTL_SIGNAL_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

namespace avrtl
{
	static constexpr uint32_t timerPrescaler = 8;
	static constexpr uint32_t timerFreqKhz = ( F_CPU / timerPrescaler ) / 1024;
	
	static inline uint32_t ticksToMicroseconds(uint32_t nTicks)
	{
		return (nTicks * 1024) / timerFreqKhz;
	}
	
	static inline uint32_t microsecondsToTicks(uint32_t us)
	{
		return (us * timerFreqKhz) / 1024;
	}

	static void DelayTimerTicks(uint32_t tickCount)
	{
		uint8_t oldSREG = SREG;
		cli();
		
		uint32_t m=0;
		uint8_t t=TCNT0, t0=t;
		
#define UPDATE_CLOCK_COUNTER() do{ uint8_t t2=TCNT0; if(t2<t) ++m; t=t2; }while(0)
#define CLOCK_ELAPSED() ((m*256+t)-t0)

		UPDATE_CLOCK_COUNTER();
		while( CLOCK_ELAPSED() < tickCount ) { UPDATE_CLOCK_COUNTER(); }

#undef UPDATE_CLOCK_COUNTER
#undef CLOCK_ELAPSED

		SREG = oldSREG;
	}

	static void DelayMicroseconds(uint32_t timeout)
	{
		DelayTimerTicks( microsecondsToTicks(timeout) );
	}
	
	template<typename PinT>
	static void blink(PinT& led, int N=20)
	{
		for(int j=1;j<=N;j++)
		{
			led = j&1;
			DelayMicroseconds(100000);
		}
	}

	// Freq: frequency in Hz
	// value: fraction (in the range [0;65535]) of the period where the level is high
	template<uint32_t Freq, typename PinT>
	static inline void pulsePWM(PinT& tx, uint16_t value, uint32_t duration)
	{
		uint32_t periodTicks = F_CPU / ( Freq * timerPrescaler );
		uint32_t highPeriod = ( periodTicks * value ) >> 16;
		uint32_t lowPeriod = periodTicks - highPeriod;
		
		duration = microsecondsToTicks(duration);
		
		uint8_t oldSREG = SREG;
		cli();

		uint32_t switchTime = periodTicks;
		uint32_t m=0;
		uint8_t t=TCNT0, t0=t;

		#define UPDATE_CLOCK_COUNTER() do{ uint8_t t2=TCNT0; if(t2<t) ++m; t=t2; }while(0)
		#define CLOCK_ELAPSED() ((m*256+t)-t0)

		UPDATE_CLOCK_COUNTER();
		bool lvl = false;
		tx.Set(lvl);
		uint32_t ts = CLOCK_ELAPSED();
		while( ts < duration )
		{
			UPDATE_CLOCK_COUNTER();
			ts = CLOCK_ELAPSED();
			while( ts<switchTime ) { UPDATE_CLOCK_COUNTER(); ts=CLOCK_ELAPSED(); }
			lvl = !lvl;
			tx.Set(lvl);
			switchTime += periodTicks;
			UPDATE_CLOCK_COUNTER();
			ts = CLOCK_ELAPSED();
		}
		
		SREG=oldSREG;
	}

	template<typename PinT>
	static uint32_t PulseIn(PinT& p, bool lvl, uint32_t timeout, uint16_t* gap=0) 
	{
		timeout = microsecondsToTicks(timeout);

		uint8_t oldSREG = SREG;
		cli();
		
		uint32_t m=0;
		uint8_t t=TCNT0, t0=t;
		
		#define UPDATE_CLOCK_COUNTER() do{ uint8_t t2=TCNT0; if(t2<t) ++m; t=t2; }while(0)
		#define CLOCK_ELAPSED() ((m*256+t)-t0)

		UPDATE_CLOCK_COUNTER();
		uint32_t ts = CLOCK_ELAPSED();
		while( ts<timeout && p.Get()!=lvl ) { UPDATE_CLOCK_COUNTER(); ts=CLOCK_ELAPSED(); }
		if( ts >= timeout ) { SREG=oldSREG; return 0; }
		
		UPDATE_CLOCK_COUNTER();
		uint32_t te = CLOCK_ELAPSED();
		while( (te-ts)<timeout && p.Get()==lvl ) { UPDATE_CLOCK_COUNTER(); te=CLOCK_ELAPSED(); }
		
		SREG=oldSREG;
		if( gap != 0 ) { *gap = ts; }
		if( (te-ts) >= timeout ) { return 0; }
		else { return ticksToMicroseconds(te-ts); }

		#undef UPDATE_CLOCK_COUNTER
		#undef CLOCK_ELAPSED
	}

	template<typename PinT>
	static uint16_t RecordSignal(PinT& p, uint32_t timeout, uint16_t nSamples, uint16_t* signal) 
	{
		timeout = microsecondsToTicks(timeout);
		bool lvl = p.Get();

		uint8_t oldSREG = SREG;
		cli();
		uint32_t m=0;
		uint8_t t=TCNT0, t0=t;

#define UPDATE_CLOCK_COUNTER() do{ uint8_t t2=TCNT0; if(t2<t){ ++m; } t=t2; }while(0)
#define CLOCK_ELAPSED() ((m*256+t)-t0)

		UPDATE_CLOCK_COUNTER();
		uint32_t te = CLOCK_ELAPSED();
		for(int i=0;i<nSamples;i++)
		{
			uint32_t ts = te;
			while( te<timeout && p.Get()!=lvl ) { UPDATE_CLOCK_COUNTER(); te=CLOCK_ELAPSED(); }
			if( te >= timeout )
			{
				SREG=oldSREG;
				return i;
			}
			UPDATE_CLOCK_COUNTER();
			signal[i] = te-ts;
			lvl = !lvl;
		}
		SREG=oldSREG;
		return nSamples;

#undef UPDATE_CLOCK_COUNTER
#undef CLOCK_ELAPSED
	}
	
}

#endif
