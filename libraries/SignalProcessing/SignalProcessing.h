#pragma once

#include <stdint.h>
#include "TimeScheduler/TimeScheduler.h"

template<typename _WallClockT = int16_t >
struct SignalProcessing32
{
	using WallClockT = _WallClockT;
	TimeSchedulerT< avrtl::AvrTimer0, WallClockT > m_ts;
	bool m_started;
	
	struct ScopedSignalProcessing
	{
		SignalProcessing* m_sp;
		bool m_owner;
		inline ScopedSignalProcessing(SignalProcessing* sp)
			: m_sp( sp )
			, m_owner( ! m_sp->m_started )
		{
			if(m_owner) { m_sp->m_started = true; m_sp->m_ts.start(); }
		}
		inline ~ScopedSignalProcessing()
		{
			if(m_owner) { m_sp->m_ts.stop();  m_sp->m_started = false; }
		}
	};
	
#define SCOPED_SIGNAL_PROCESSING ScopedSignalProcessing __scopedSignalProcessing(this)
	
	inline SignalProcessing() : m_ts(), m_started(false) {}
	
	void DelayTimerTicksFast(WallClockT tickCount)
	{
		m_ts.execFast( tickCount , [](){} );
	}

	static void DelayMicrosecondsFast(WallClockT timeoutMicrosec)
	{
		m_ts.exec( timeoutMicrosec , [](){} );
	}
	
	static void DelayMicroseconds(WallClockT timeout)
	{
		SCOPED_SIGNAL_PROCESSING;
		m_ts.reset();
		DelayMicrosecondsFast( timeout );
	}

	template<typename PinT>
	static void blink(PinT& led, int N=20, WallClockT period=100000)
	{
		SCOPED_SIGNAL_PROCESSING;
		m_ts.reset();
		for(int j=1;j<=N;j++)
		{
			m_ts.exec( period , [&led](){ led = ( ( j & 1 ) == 1 ); } );
		}
	}

	// in ticks, not in microSeconds
	template<uint32_t CycleUSec, uint32_t HighPeriodUSec, typename PinT>
	static inline void pulsePWMFast(PinT tx, uint16_t nCycles)
	{
		static constexpr uint32_t lowPeriodUSec = CycleUSec - HighPeriodUSec;
		m_ts.reset();
		for(;nCycles;--nCycles)
		{
			m_ts.exec( HighPeriodUSec, [](){ tx.Set(true); } );
			m_ts.exec( lowPeriodUSec, [](){ tx.Set(false); } );
		}
	}
	
	template<uint16_t CycleUSec,  uint16_t HighPeriodUSec, typename PinT>
	static inline void pulsePWM(PinT tx, uint32_t durationUSec)
	{
		SCOPED_SIGNAL_PROCESSING;
		pulsePWMFast<CycleUSec,HighPeriodUSec>( tx, durationUSec/CycleUSec );
	}

	template<typename PinT>
	static inline void setLineFlatFast(PinT tx, bool state, uint32_t durationUSec)
	{
		m_ts.reset();
		m_ts.exec( durationUSec , [&tx,=state]()
			{
				tx.Set(state); // start signal ASAP, do initial computation during high state			
			});
	}

	template<uint32_t freq, uint16_t fraction, typename PinT>
	static inline void setLinePWMFast(PinT tx, bool state, uint32_t duration)
	{
		static constexpr uint32_t cycleUSec = 1000000UL/freq;
		static constexpr uint32_t highUSec = (cycleUSec * fraction) >> 16;
		if(state)
		{
			pulsePWMFast<cycleUSec,highUSec>( tx, durationUSec/CycleUSec );
		}
		else
		{
			setLineFlatFast(tx,false,duration);
		}
	}

	template<typename PinT>
	static uint32_t PulseInFast(PinT p, bool lvl, uint32_t timeout, uint16_t* gap=nullptr) 
	{
		timeout = m_ts.m_timerhw.microsecondsToTicks(timeout);
		
		m_ts.reset();
		WallClockT ts = 0;
		WallClockT te = 0;
		m_ts.loopFast( timeout , [&lvl,&ts,&te,&p](WallClockT t)
			{
				if( ts==0 )
				{
					if( p.Get()==lvl ) { ts = t; }
				}
				else if( te==0 )
				{
					if( p.Get()!=lvl ) { te = t; }
				}
			} );		
		if( gap != nullptr ) { *gap = ts; }
		if( (te-ts) >= timeout ) { return 0; }
		else { return m_ts.m_timerhw.ticksToMicroseconds(te-ts); }
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
		timeout = m_ts.m_timerhw.microsecondsToTicks(timeout);
		WallClockT ts = 0;
		int i=0;
		bool lvl = p.Get();
		m_ts.reset();
		m_ts.loopFast( timeout , [&lvl,&ts,&p,&i,=signal](WallClockT t)
			{
				if( p.Get() != lvl )
				{
					lvl = ! lvl;
					int32_t len = t - ts;
					ts = t;
					if(i<nSamples) { signal[i++] = len; }
				}
			});
		return i;
	}
	
	template<typename PinT>
	static uint16_t RecordSignal(PinT p, uint32_t timeout, uint16_t nSamples, uint16_t* signal) 
	{
		SCOPED_SIGNAL_PROCESSING;
		uint16_t r = RecordSignalFast( p, timeout, nSamples, signal );
		return r;
	}
	
}

