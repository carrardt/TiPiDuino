#pragma once

#include <avr/io.h>

namespace avrtl
{

template<class TimerCounterType>
struct TimeOutWatcher
{
  int32_t m_ticks = 0;
  TimerCounterType m_last = 0;
  inline void update( TimerCounterType cur )
  {
    m_ticks -= (TimerCounterType)( cur - m_last );
    m_last = cur;
  }
  inline bool exhausted() const { return m_ticks < 0 ; }
};

/*!
 * Timer 0 is an 8-bit timer available on all atmel chips with selectable 1 or 8 prescaler.
 * This class represents the HW resources for this timer.
 */
struct AvrTimer0HW
{
	static constexpr uint32_t TimerCounterResolution = 256;
	static constexpr uint32_t TimerCounterMax = TimerCounterResolution - 1;
	using TimerCounterType = uint8_t;

	inline void pushState(uint32_t prescalerValue)
	{
		saved_TCCR0A = TCCR0A;
		saved_TCCR0B = TCCR0B;
#   ifdef TIMSK0
		saved_TIMSK0 = TIMSK0;
		TIMSK0 = 0;
#   endif
		TCCR0A = 0;
		switch(prescalerValue)
		{
		    case 1    : TCCR0B = 0b00000001; break;
		    case 8    : TCCR0B = 0b00000010; break;
		    case 64   : TCCR0B = 0b00000011; break;
		    case 256  : TCCR0B = 0b00000100; break;
		    case 1024 : TCCR0B = 0b00000101; break;
		    default   : TCCR0B = 0b00000000; break; // stopped
		}
	}
	
	inline void popState() const
	{
		TCCR0A = saved_TCCR0A;
		TCCR0B = saved_TCCR0B;
#ifdef TIMSK0
		TIMSK0 = saved_TIMSK0;
#endif
	}
	
  static inline void resetCounter()
  {
    TCNT0 = 0;
  }

	static inline TimerCounterType counter() { return TCNT0; }
	
	uint8_t saved_TCCR0A, saved_TCCR0B;
#ifdef TIMSK0
	uint8_t saved_TIMSK0;
#endif
};

// Warning! : it seems it doesn't work on ATtiny85, needs investigation. for ATtiny, use only Timer0.
#if defined(TCCR1A) && defined(TCCR1B) && defined(TCCR1C) && defined(TIMSK1)
#define AVR_HAS_TIMER1HW 1 
struct AvrTimer1HW
{
	using TimerCounterType = uint16_t;
	static constexpr uint32_t TimerCounterResolution = 65536;
	static constexpr uint32_t TimerCounterMax = TimerCounterResolution - 1;

  static constexpr uint32_t ExternalClockRising = 3;
  static constexpr uint32_t ExternalClockFalling = 5;

  static inline void resetCounter()
  {
    TCNT1 = 0;
  }

	inline void pushState(uint32_t prescalerValue)
	{
		saved_TCCR1A = TCCR1A;
		saved_TCCR1B = TCCR1B;
		saved_TCCR1C = TCCR1C;
		saved_TIMSK1 = TIMSK1;
		TIMSK1 = 0;
		TCCR1A = 0;
		TCCR1C = 0;
		switch( prescalerValue )
		{
			case 1    : TCCR1B = 0b00000001; break;
			case 8    : TCCR1B = 0b00000010; break;
			case 64   : TCCR1B = 0b00000011; break;
			case 256  : TCCR1B = 0b00000100; break;
			case 1024 : TCCR1B = 0b00000101; break;
			case ExternalClockRising : TCCR1B = 0b00000111; break;
			case ExternalClockFalling : TCCR1B = 0b00000110; break;
			default   : TCCR1B = 0b00000000; break; // sopped
		}
    resetCounter();
	}

	inline void popState() const
	{
		TCCR1A = saved_TCCR1A;
		TCCR1B = saved_TCCR1B;
		TCCR1C = saved_TCCR1C;
		TIMSK1 = saved_TIMSK1;
	}

	static inline TimerCounterType counter() { return TCNT1; }

	uint8_t saved_TCCR1A;
	uint8_t saved_TCCR1B;
	uint8_t saved_TCCR1C;
	uint8_t saved_TIMSK1;
};
#endif

template<typename _TimerHW, uint32_t _TimerPrescaler>
struct AvrTimer
{	
	// the only usable value
	using TimerHW = _TimerHW;
	using TimerCounterType = typename TimerHW::TimerCounterType;
	static constexpr uint32_t TimerCounterResolution = TimerHW::TimerCounterResolution;
	static constexpr uint32_t TimerCounterMax = TimerHW::TimerCounterResolution;

	static constexpr uint32_t TimerPrescaler = _TimerPrescaler;
	static constexpr uint32_t ClockMhz = F_CPU / 1000000UL;
	static constexpr uint32_t NanoSecPerTick = ( 1000UL * TimerPrescaler ) / ClockMhz;
	static constexpr uint32_t TicksPerMilliSec = ( F_CPU / TimerPrescaler ) / 1000UL;
	static constexpr uint32_t TicksPerSecond = F_CPU / TimerPrescaler;
	static constexpr uint32_t MaxExecMicroSec = ( TimerCounterMax * NanoSecPerTick ) / 1000UL;

	inline TimerCounterType start()
	{
		m_timerhw.pushState(TimerPrescaler);
		return counter();
	}
	
	inline void resetCounter()
	{
	  m_timerhw.resetCounter();
	}
	
	inline TimerCounterType counter() const
	{
		return m_timerhw.counter();
	}
	
	inline void stop() const
	{
		m_timerhw.popState();
	}

	static constexpr uint32_t ticksToMicroseconds(uint32_t nTicks)
	{
		return ( nTicks * TimerPrescaler ) / ClockMhz;
	}

	static constexpr uint32_t microsecondsToTicks(uint32_t us)
	{
		return ( us * ClockMhz ) / TimerPrescaler;
	}

	inline void delay(int32_t ticks) const
	{
		TimerCounterType t = counter();
		do
		{
			TimerCounterType t2 = counter();
			ticks -= (TimerCounterType)( t2 - t );
			t = t2;
		} while( ticks > 0 );
	}

	inline void delayMicroseconds(int32_t uS) const
	{
	  this->delay( microsecondsToTicks(uS) );
  }

  inline TimeOutWatcher<TimerCounterType> startTimeout(int32_t uS)
  {
    TimeOutWatcher<TimerCounterType> timeout;
    timeout.m_ticks = microsecondsToTicks(uS);
    timeout.m_last = counter();
    return timeout;
  }
  
  inline bool isExhausted( TimeOutWatcher<TimerCounterType> & timeout )
  {
    timeout.update( counter() );
    return timeout.exhausted();
  }

	TimerHW m_timerhw;
};

using AvrTimer0            = AvrTimer<AvrTimer0HW,8>;
using AvrTimer0NoPrescaler = AvrTimer<AvrTimer0HW,1>;

#ifdef AVR_HAS_TIMER1HW
using AvrTimer1            = AvrTimer<AvrTimer1HW,8>;
using AvrTimer1NoPrescaler = AvrTimer<AvrTimer1HW,1>;
#endif

template<class TimerT>
struct ScopedTimer
{
  TimerT & m_timer;
  inline ScopedTimer(TimerT & tm) : m_timer(tm) { m_timer.start(); }
  inline ~ScopedTimer() { m_timer.stop(); }
};


inline void delayMicroseconds(uint32_t us)
{
	AvrTimer0 timer;
	timer.start();
	timer.delay( timer.microsecondsToTicks(us) );
	timer.stop();
}

inline void delay(unsigned long ms)
{
	delayMicroseconds( static_cast<uint32_t>(ms) * 1000 );
}

} // namespace avrtl

