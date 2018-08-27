#ifndef __TiDuino_TimeScheduler_h
#define __TiDuino_TimeScheduler_h

#include <avr/io.h>

namespace avrtimer
{

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
#ifdef TIMSK0
		saved_TIMSK0 = TIMSK0;
		TIMSK0 = 0;
#endif
		TCCR0A = 0;
		switch(prescalerValue)
		{
		    case 1    : TCCR0B = 0b00000001; break;
		    case 8    : TCCR0B = 0b00000010; break;
		    case 64   : TCCR1B = 0b00000011; break;
		    case 256  : TCCR1B = 0b00000100; break;
		    case 1024 : TCCR1B = 0b00000101; break;
		    default   : TCCR0B = 0b00000000; break; // stopped
		}
	}
	
	inline void popState()
	{
		TCCR0A = saved_TCCR0A;
		TCCR0B = saved_TCCR0B;
#ifdef TIMSK0
		TIMSK0 = saved_TIMSK0;
#endif
	}
	
	static inline TimerCounterType counter() { return TCNT0; }
	
	uint8_t saved_TCCR0A, saved_TCCR0B;
#ifdef TIMSK0
	uint8_t saved_TIMSK0;
#endif
};

// Warning! : it seems it doesn't work on ATtiny85, needs investigation. for ATtiny, use only Timer0.
#if defined(TCCR1A) && defined(TCCR1B) && defined(TCCR1C) && defined(TIMSK1) 
struct AvrTimer1HW
{
	using TimerCounterType = uint16_t;
	static constexpr uint32_t TimerCounterResolution = 65536;
	static constexpr uint32_t TimerCounterMax = TimerCounterResolution - 1;

	inline void pushState(uint32_t prescalerValue)
	{
		saved_TCCR1A = TCCR1A;
		saved_TCCR1B = TCCR1B;
		saved_TCCR1C = TCCR1C;
		saved_TIMSK1 = TIMSK1;
		TIMSK1 = 0;
		TCCR1A = 0;
		TCCR1C = 0;
		switch( TimerPrescaler )
		{
			case 1    : TCCR1B = 0b00000001; break;
			case 8    : TCCR1B = 0b00000010; break;
			case 64   : TCCR1B = 0b00000011; break;
			case 256  : TCCR1B = 0b00000100; break;
			case 1024 : TCCR1B = 0b00000101; break;
			default   : TCCR1B = 0b00000000; break; // sopped
		}
	}

	inline void popState()
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
	static constexpr uint32_t TimerPrescaler = _TimerPrescaler;
	static constexpr uint32_t ClockMhz = F_CPU / 1000000UL;
	static constexpr uint32_t NanoSecPerTick = ( 1000UL * TimerPrescaler ) / ClockMhz;
	static constexpr uint32_t TicksPerMilliSec = ( F_CPU / TimerPrescaler ) / 1000UL;
	static constexpr uint32_t TicksPerSecond = F_CPU / TimerPrescaler;
	static constexpr uint32_t MaxExecMicroSec = ( TimerCounterMax * NanoSecPerTick ) / 1000UL;

	inline TimerCounterType start()
	{
		m_timerhw.pushState(TimerPrescaler);
	}
	
	inline TimerCounterType counter() const
	{
		return m_timerhw.counter();
	}
	
	inline stop()
	{
		m_timerhw.popState();
	}

	TimerHW m_timerhw;
};

	
} // namespace avrtimer

using AvrTimer0 = avrtimer::AvrTimer<AvrTimer0HW,8>;
using AvrTimer0NoPrescaler = avrtimer::AvrTimer<AvrTimer0HW,1>;
using AvrTimer1 = avrtimer::AvrTimer<AvrTimer1HW,8>;
using AvrTimer1NoPrescaler = avrtimer::AvrTimer<AvrTimer1HW,1>;

// --- Debugging features ---
template<typename WallClockT, bool DebugMode>
struct TimeSchedulerDebug
{
	static constexpr int MaxTimings = 16;
	static constexpr int DbgOutputPeriod = 16;
	
	WallClockT m_timings[MaxTimings];
	uint8_t m_i;
	uint8_t m_dbgcnt;
	
	inline TimeSchedulerDebug()
		: m_dbgcnt(0)
	{ 
		reset();
	}
	
	inline void reset()
	{
		m_i=0;
		for(int j=0;j<MaxTimings;j++) { m_timings[j]=0; }
	}
	inline void setTiming(WallClockT t) { m_timings[m_i]=t; }
	inline void updateTiming(WallClockT t) { if(t>m_timings[m_i]) m_timings[m_i]=t; }
	inline void next() { ++m_i; }
	
	template<typename OStreamT>
	inline bool printDebugInfo(OStreamT& cout, uint32_t tickTime)
	{
		++ m_dbgcnt;
		if( m_dbgcnt == DbgOutputPeriod )
		{
			cout<<"\n>"<<m_i<<"\n";
			for(int i=0;i<m_i;i++)
			{
				uint32_t T = m_timings[i];
				T = ( T * tickTime ) / 1000;
				cout<<T<<' ';
			}
			cout<<'\n';
			m_dbgcnt = 0;
		}
		return m_dbgcnt==0;
	}

};

template<typename WallClockT>
struct TimeSchedulerDebug<WallClockT, false>
{
	static inline void reset() {}
	static inline void setTiming(WallClockT t) {}
	static inline void updateTiming(WallClockT t) {}
	static inline void next() {}
	template<typename OStreamT> static inline bool printDebugInfo(const OStreamT&,uint32_t) { return false; }
};

template<typename _BaseTimer = AvrTimer0, typename _WallClockT = int16_t, bool DebugMode = false>
struct TimeSchedulerT
{	
	using BaseTimerT = _BaseTimer;
	using CounterT = typename BaseTimerT::TimerCounterType;
	using WallClockT = _WallClockT;
	using DebuggerT = TimeSchedulerDebug<WallClockT,DebugMode>;

	static constexpr WallClockT MaxWallClock = ( 1ULL << (sizeof(WallClockT)*8-1) ) - 1;
	static constexpr WallClockT MaxLoopMilliSec = MaxWallClock / BaseTimerT::TicksPerMilliSec ;
		
	// in microseconds
	static uint32_t maxFuncTime() { return BaseTimerT::MaxExecMicroSec; }
	
	// in nanoseconds
	static uint32_t tickTime() { return BaseTimerT::NanoSecPerTick; }
	
	// in milliseconds
	static uint32_t maxCycleTime() { return MaxLoopMilliSec; }
	
	inline TimeSchedulerT()
		: m_wallclock(0)
		, m_t(0)  {}
	
	inline void start()
	{
		m_wallclock = 0;
		m_t = timer.start();
	}

	inline void reset()
	{
		m_wallclock = 0;
		m_t = timer.counter();
	}

	inline void stop()
	{
		timer.stop();
	}

	inline WallClockT wallclock()
	{
		CounterT t2 = timer.counter();
		m_wallclock += (CounterT)( t2 - m_t );
		m_t = t2;
		return m_wallclock;
	}
	
	template<typename FuncT>
	inline void execFast( WallClockT t, FuncT f )
	{
		f();
		while( wallclock() < t );
		m_wallclock -= t;
	}

	template<typename FuncT>
	inline void loopFast( WallClockT t, FuncT f )
	{
		WallClockT w;
		while( (w=wallclock()) < t ) { f( w ); }
		m_wallclock -= t;
	}

	template<typename FuncT>
	inline void exec( WallClockT t, FuncT f )
	{
		t = ( t * BaseTimerT::ClockMhz ) / BaseTimerT::TimerPrescaler;
		f();
		m_debugger.setTiming(wallclock());
		m_debugger.next();
		while( wallclock() < t );
		m_wallclock -= t;
	}

	template<typename FuncT>
	inline void loop( WallClockT t, FuncT f )
	{
		t = ( t * BaseTimerT::ClockMhz ) / BaseTimerT::TimerPrescaler;
		WallClockT w;
		while( (w=wallclock()) < t )
		{ 
			WallClockT usec = ( w * BaseTimerT::TimerPrescaler ) / BaseTimerT::ClockMhz ;
			f( usec );
			if(DebugMode) {	WallClockT w2 = wallclock(); m_debugger.updateTiming(w2-w); }
		}
		m_wallclock -= t;
		m_debugger.next();
	}

	template<typename OStreamT>
	inline bool printDebugInfo(OStreamT& cout)
	{
		if( m_debugger.printDebugInfo(cout,tickTime()) )
		{
			reset();
		}
	}
	
	inline void endCycle()
	{
		m_debugger.reset();
	}
	
	BaseTimerT timer;
	WallClockT m_wallclock;
	CounterT m_t;
	DebuggerT m_debugger;
};

// backward compatibilty
using TimeScheduler = TimeSchedulerT<>;

#endif
