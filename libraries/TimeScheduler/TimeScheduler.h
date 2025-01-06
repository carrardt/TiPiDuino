#pragma once

#include <avr/io.h>
#include <AvrTL/timer.h>

// TODO: replace 'Fast' suffix by 'Ticks' suffix

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

template<typename _BaseTimer = avrtl::AvrTimer0, typename _WallClockT = int16_t, bool DebugMode = false>
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
		m_t = m_timer.start();
	}

	inline void reset()
	{
		m_wallclock = 0;
		m_t = m_timer.counter();
	}

	inline void stop()
	{
		m_timer.stop();
	}

	inline WallClockT wallclock()
	{
		CounterT t2 = m_timer.counter();
		m_wallclock += (CounterT)( t2 - m_t );
		m_t = t2;
		return m_wallclock;
	}
	
	inline void rewindWallClock(WallClockT t)
	{
		m_wallclock -= t;
	}
	
	// works in timer tick time unit
	template<typename FuncT>
	inline void execFast( WallClockT t, FuncT f )
	{
		f();
		while( wallclock() < t );
		rewindWallClock( t );
	}

	// works in timer tick time unit
	template<typename FuncT>
	inline void loopFast( WallClockT t, FuncT f )
	{
		WallClockT w;
		while( (w=wallclock()) < t ) { f( w ); }
		rewindWallClock( t );
	}

	// works in uS time unit
	template<typename FuncT>
	inline void exec( WallClockT t, FuncT f )
	{
		t = ( t * BaseTimerT::ClockMhz ) / BaseTimerT::TimerPrescaler;
		f();
		m_debugger.setTiming(wallclock());
		m_debugger.next();
		while( wallclock() < t );
		rewindWallClock( t );
	}

	// works in uS time unit
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
		rewindWallClock( t );
		m_debugger.next();
	}


	inline void delay(WallClockT t)
	{
		exec( t , [](){} );
	}

	template<typename OStreamT>
	inline bool printDebugInfo(OStreamT& cout)
	{
		if( m_debugger.printDebugInfo(cout,tickTime()) )
		{
			reset();
		}
		return true;
	}
	
	inline void endCycle()
	{
		m_debugger.reset();
	}
	
	BaseTimerT m_timer;
	WallClockT m_wallclock;
	CounterT m_t;
	DebuggerT m_debugger;
};

// backward compatibilty
using TimeScheduler = TimeSchedulerT<>;


