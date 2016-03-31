#ifndef __AVRTL_SIGNAL_H
#define __AVRTL_SIGNAL_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

namespace avrtl
{
	static constexpr uint32_t timerPrescaler = 8;
	static constexpr uint32_t clockMhz = F_CPU / 1000000UL;
	extern uint8_t saved_SREG;
	extern uint8_t saved_TCCR0B;
	extern bool signalProcessingModeActive;
	
#define INIT_CLOCK_COUNTER() 	uint8_t _t=TCNT0,_t0=_t; int32_t _m=-_t0
#define INIT_CLOCK_COUNTER16() 	uint8_t _t=TCNT0,_t0=_t; int16_t _m=-_t0
#define REWIND_CLOCK_COUNTER(x) do{ _m -= (x); }while(0)
#define RESET_CLOCK_COUNTER() do{ _m=0; _t=TCNT0; }while(0)
#define UPDATE_CLOCK_COUNTER() 	do{ uint8_t _t2=TCNT0; if(_t2<_t)_m+=256; _t=_t2; }while(0)
#define CLOCK_ELAPSED() 		(_m+_t)

	static constexpr uint32_t ticksToMicroseconds(uint32_t nTicks)
	{
		return ( nTicks * timerPrescaler ) / clockMhz;
	}

	static constexpr uint32_t microsecondsToTicks(uint32_t us)
	{
		return ( us * clockMhz ) / timerPrescaler;
	}

	static constexpr uint16_t pwmval(float dutyCyclesFraction)
	{
		return 65535.75f * dutyCyclesFraction;
	}

	struct SignalProcessingMode
	{
		bool m_owner;
		inline void start()
		{
			m_owner = (saved_TCCR0B == 0);
			if( m_owner )
			{
				saved_SREG = SREG;
				cli();
				saved_TCCR0B = TCCR0B;
				// Sets the timer prescale factor to 8; 1/2 microsecond per tick @16Mhz
				TCCR0B = (TCCR0B & 0b11111000) | 0b010;
			}
		}
		inline void stop()
		{
			if( m_owner )
			{
				TCCR0B = saved_TCCR0B;
				SREG = saved_SREG;
				saved_TCCR0B = 0;
			}
		}
	};

	struct ScopedSignalProcessingMode : public SignalProcessingMode
	{
		inline ScopedSignalProcessingMode() { start(); }
		inline ~ScopedSignalProcessingMode() { stop(); }
	};
	
#define SCOPED_SIGNAL_PROCESSING avrtl::ScopedSignalProcessingMode __scopedSignalProcessingMode

	static void DelayTimerTicksFast(uint32_t tickCount)
	{
		INIT_CLOCK_COUNTER();
		UPDATE_CLOCK_COUNTER();
		while( CLOCK_ELAPSED() < tickCount ) { UPDATE_CLOCK_COUNTER(); }
	}

	static void DelayMicrosecondsFast(uint32_t timeout)
	{
		DelayTimerTicksFast( microsecondsToTicks(timeout) );
	}
	
	static void DelayMicroseconds(uint32_t timeout)
	{
		SCOPED_SIGNAL_PROCESSING;
		DelayTimerTicksFast( microsecondsToTicks(timeout) );
	}

	template<typename PinT>
	static void blink(PinT& led, int N=20)
	{
		SCOPED_SIGNAL_PROCESSING;
		for(int j=1;j<=N;j++)
		{
			led = j&1;
			DelayMicroseconds(100000);
		}
	}

	template<uint16_t CycleTicks, typename PinT, typename FuncT>
	static inline void loopPWM(PinT tx, FuncT updateFunc)
	{
		SCOPED_SIGNAL_PROCESSING;
		
		uint16_t HighPeriodTicks = updateFunc();
		uint16_t LowPeriodTicks = 0;
		uint16_t nextSwitchTime = 0;		
		uint16_t wallClock = 0;
		
		INIT_CLOCK_COUNTER16();		
		tx.Set(true);
		while( HighPeriodTicks < CycleTicks )
		{
			// shift timer back so that 16bits wallclock is always enough
			REWIND_CLOCK_COUNTER(LowPeriodTicks);
			
			do {
				UPDATE_CLOCK_COUNTER();
				wallClock = CLOCK_ELAPSED();
			} while( wallClock < HighPeriodTicks );

			// as soon as we detect high time is elapsed, set to low
			tx.Set(false);
			
			// keep timer in sync
			UPDATE_CLOCK_COUNTER();
			
			// shift timer back so that 16bits wallclock is always enough
			REWIND_CLOCK_COUNTER(HighPeriodTicks);
			
			// update timings
			LowPeriodTicks = CycleTicks - HighPeriodTicks;
			HighPeriodTicks = updateFunc();
			bool highState = (HighPeriodTicks>0);
			
			do {
				UPDATE_CLOCK_COUNTER();
				wallClock = CLOCK_ELAPSED();
			} while( wallClock<LowPeriodTicks );

			// as soon as we detect low time is elapsed, set to high
			tx.Set(highState);
		}
	}

	template<uint16_t CycleTicks, typename PinT, typename Func1T, typename Func2T>
	static inline void loopDualPWM(PinT dualPin, Func1T updateFunc1, Func2T updateFunc2)
	{
		SCOPED_SIGNAL_PROCESSING;		
		uint16_t HighPeriodTicks1 = updateFunc1();
		uint16_t HighPeriodTicks2 = updateFunc2();
		bool pinOrder = ( HighPeriodTicks1 >= HighPeriodTicks2 );
		uint16_t HighPeriodTicks = 0;
		uint16_t LowPeriodTicks = 0;
		uint16_t nextSwitchTime = 0;		
		uint16_t wallClock = 0;
		bool highState = (HighPeriodTicks1!=0) && (HighPeriodTicks2!=0) ;
		
		dualPin.SelectAllPins();
		dualPin.Set(true);
		INIT_CLOCK_COUNTER16();
		while( highState )
		{
			// shift timer back so that 16bits wallclock is always enough
			REWIND_CLOCK_COUNTER(LowPeriodTicks);
			
			HighPeriodTicks = pinOrder ? HighPeriodTicks2 : HighPeriodTicks1;
			dualPin.SelectPin(pinOrder);
			do {
				UPDATE_CLOCK_COUNTER();
				wallClock = CLOCK_ELAPSED();
			} while( wallClock < HighPeriodTicks );
			dualPin.Set( false );

			HighPeriodTicks = pinOrder ? HighPeriodTicks1 : HighPeriodTicks2;
			dualPin.SelectPin(!pinOrder);
			do {
				UPDATE_CLOCK_COUNTER();
				wallClock = CLOCK_ELAPSED();
			} while( wallClock < HighPeriodTicks );
			dualPin.Set( false );
			
			// keep timer in sync
			UPDATE_CLOCK_COUNTER();
			
			// shift timer back so that 16bits wallclock is always enough
			REWIND_CLOCK_COUNTER(HighPeriodTicks);
			
			// update timings
			LowPeriodTicks = CycleTicks - HighPeriodTicks;

			HighPeriodTicks1 = updateFunc1();
			UPDATE_CLOCK_COUNTER();
			HighPeriodTicks2 = updateFunc2();
			dualPin.SelectAllPins();
			highState = (HighPeriodTicks1!=0) && (HighPeriodTicks2!=0) ;

			do {
				UPDATE_CLOCK_COUNTER();
				wallClock = CLOCK_ELAPSED();
			} while( wallClock<LowPeriodTicks );

			// as soon as we detect low time is elapsed, set to high
			dualPin.Set(highState);
		}
	}

	// in ticks, not in microSeconds
	template<uint16_t CycleTicks, uint16_t HighPeriodTicks, typename PinT>
	static inline void pulsePWMFastTicks(PinT tx, uint16_t nCycles)
	{
		INIT_CLOCK_COUNTER16();
		tx.Set(true); // start signal ASAP, do initial computation during high state
		UPDATE_CLOCK_COUNTER();
		uint16_t lowPeriod = CycleTicks - HighPeriodTicks;
		uint16_t nextSwitchTime = 0;		
		for(;nCycles;--nCycles)
		{
			tx.Set(true);
			nextSwitchTime += HighPeriodTicks;
			uint16_t wallClock;
			do {
				UPDATE_CLOCK_COUNTER();
				wallClock = CLOCK_ELAPSED();
			} while( wallClock<nextSwitchTime );
			tx.Set(false);
			nextSwitchTime += lowPeriod;
			do {
				UPDATE_CLOCK_COUNTER();
				wallClock = CLOCK_ELAPSED();
			} while( wallClock<nextSwitchTime );
		}
		//tx.Set(false);
	}

	template<uint16_t CycleUSec, uint16_t HighPeriodUSec, typename PinT>
	static inline void pulsePWMFast(PinT tx, uint16_t durationUSec)
	{
		pulsePWMFastTicks<microsecondsToTicks(CycleUSec),microsecondsToTicks(HighPeriodUSec)>( tx, durationUSec / CycleUSec );
	}
	
	template<uint16_t CycleUSec,  uint16_t HighPeriodUSec, typename PinT>
	static inline void pulsePWM(PinT tx,uint16_t duration)
	{
		SCOPED_SIGNAL_PROCESSING;
		pulsePWMFast<CycleUSec,HighPeriodUSec>( tx, duration );
	}

	template<uint16_t CycleUSec,  uint16_t HighPeriodUSec, typename PinT>
	static inline void longPulsePWM(PinT tx,uint32_t durationUSec)
	{
		SCOPED_SIGNAL_PROCESSING;
		pulsePWMFastTicks<microsecondsToTicks(CycleUSec),microsecondsToTicks(HighPeriodUSec)>( tx, durationUSec/CycleUSec );
	}

	template<typename PinT>
	static inline void pulseFast(PinT tx, bool value, uint32_t duration)
	{
		INIT_CLOCK_COUNTER();
		tx.Set(value); // start signal ASAP, do initial computation during high state
		duration = microsecondsToTicks(duration);
		uint32_t wallClock;
		do {
			UPDATE_CLOCK_COUNTER();
			wallClock = CLOCK_ELAPSED();
		} while( wallClock < duration );
		tx.Set(!value);
	}

	template<typename PinT>
	static inline void pulse(PinT tx, bool value, uint32_t duration)
	{
		SCOPED_SIGNAL_PROCESSING;
		pulseFast( tx, value, duration );
	}

	template<typename PinT>
	static inline void setLineFlatFast(PinT tx, bool state, uint32_t duration)
	{
		INIT_CLOCK_COUNTER();
		tx.Set(state); // start signal ASAP, do initial computation during high state
		duration = microsecondsToTicks(duration);
		uint32_t wallClock;
		do {
			UPDATE_CLOCK_COUNTER();
			wallClock = CLOCK_ELAPSED();
		} while( wallClock < duration );
	}

	template<uint16_t freq, uint16_t pwmValue, typename PinT>
	static inline void setLinePWMFast(PinT tx, bool state, uint32_t duration)
	{
		static constexpr uint16_t periodus = 1000000UL/38000UL;
		static constexpr uint16_t cycleTicks = F_CPU/(avrtl::timerPrescaler*freq);
		static constexpr uint16_t highTicks = ( ((uint32_t)cycleTicks)*((uint32_t)pwmValue) ) >> 16;
		if(state)
		{
			pulsePWMFastTicks<cycleTicks,highTicks>( tx, duration/periodus );
		}
		else
		{
			setLineFlatFast(tx,false,duration);
		}
	}

	template<typename PinT>
	static uint32_t PulseInFast(PinT p, bool lvl, uint32_t timeout, uint16_t* gap=0) 
	{
		INIT_CLOCK_COUNTER();
		timeout = microsecondsToTicks(timeout);

		UPDATE_CLOCK_COUNTER();
		uint32_t ts = CLOCK_ELAPSED();
		while( ts<timeout && p.Get()!=lvl ) { UPDATE_CLOCK_COUNTER(); ts=CLOCK_ELAPSED(); }
		if( ts >= timeout ) { return 0; }

		UPDATE_CLOCK_COUNTER();
		uint32_t te = CLOCK_ELAPSED();
		while( (te-ts)<timeout && p.Get()==lvl ) { UPDATE_CLOCK_COUNTER(); te=CLOCK_ELAPSED(); }
		
		if( gap != 0 ) { *gap = ts; }
		if( (te-ts) >= timeout ) { return 0; }
		else { return ticksToMicroseconds(te-ts); }
	}

	template<typename PinT>
	static uint32_t PulseIn(PinT p, bool lvl, uint32_t timeout, uint16_t* gap=0) 
	{
		SCOPED_SIGNAL_PROCESSING;
		return PulseInFast( p, lvl, timeout, gap );
	}

	template<typename PinT>
	static uint16_t RecordSignalFast(PinT p, uint32_t timeout, uint16_t nSamples, uint16_t* signal) 
	{
		timeout = microsecondsToTicks(timeout);
		uint32_t te = 0;
		int32_t ts = 0;
		int i=0;
		bool lvl = p.Get();
		INIT_CLOCK_COUNTER();
		UPDATE_CLOCK_COUNTER();
		for(;i<nSamples && te<timeout;i++)
		{
			UPDATE_CLOCK_COUNTER();
			te=CLOCK_ELAPSED();
			while( te<timeout && p.Get()==lvl ) { UPDATE_CLOCK_COUNTER(); te=CLOCK_ELAPSED(); }
			ts = te-ts;
			if( ts>65535 ) signal[i] = 65535;
			else signal[i] = ts;
			lvl = !lvl;
			ts = te;
		}
		if(te>=timeout) return i;
		else return nSamples;
	}
	
	template<typename PinT>
	static uint16_t RecordSignal(PinT p, uint32_t timeout, uint16_t nSamples, uint16_t* signal) 
	{
		SCOPED_SIGNAL_PROCESSING;
		uint16_t r = RecordSignalFast( p, timeout, nSamples, signal );
		return r;
	}

//#undef UPDATE_CLOCK_COUNTER
//#undef CLOCK_ELAPSED
//#undef INIT_CLOCK_COUNTER
}

static inline void delayMicroseconds(uint32_t us)
{
	avrtl::DelayMicroseconds(us);
}

#endif
