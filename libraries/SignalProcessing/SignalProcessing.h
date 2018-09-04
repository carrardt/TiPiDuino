#pragma once

#include <stdint.h>
#include "TimeScheduler/TimeScheduler.h"

template<typename _WallClockT = int16_t >
struct SignalProcessing
{
	using WallClockT = _WallClockT;
	
	inline SignalProcessing()
	{
		m_ts.start();
	}

	inline ~SignalProcessing()
	{
		m_ts.stop();
	}
	
	void delayMicroseconds(WallClockT timeout)
	{
		m_ts.reset();
		m_ts.delay( timeout );
	}

	template<typename PinT>
	void blink(PinT& led, int N=20, WallClockT period=100000)
	{
		m_ts.reset();
		for(int j=1;j<=N;j++)
		{
			m_ts.exec( period , [j,&led](){ led = ( ( j & 1 ) == 1 ); } );
		}
	}

	// in ticks, not in microSeconds
	template<uint32_t CycleUSec, uint32_t HighPeriodUSec, typename PinT>
	inline void pulsePWM(PinT tx, uint16_t nCycles)
	{
		static constexpr uint32_t lowPeriodUSec = CycleUSec - HighPeriodUSec;
		m_ts.reset();
		for(;nCycles;--nCycles)
		{
			m_ts.exec( HighPeriodUSec, [tx](){ tx.Set(true); } );
			m_ts.exec( lowPeriodUSec, [tx](){ tx.Set(false); } );
		}
	}
	
	template<typename PinT>
	inline void setLineFlat(PinT tx, bool state, uint32_t durationUSec)
	{
		m_ts.reset();
		m_ts.exec( durationUSec , [tx,state]()
			{
				tx.Set(state); // start signal ASAP, do initial computation during high state			
			});
	}

	template<uint32_t freq, uint16_t fraction, typename PinT>
	inline void setLinePWM(PinT tx, bool state, uint32_t durationUSec)
	{
		static constexpr uint32_t cycleUSec = 1000000UL/freq;
		static constexpr uint32_t highUSec = (cycleUSec * fraction) >> 16;
		if(state)
		{
			pulsePWM<cycleUSec,highUSec>( tx, durationUSec/cycleUSec );
		}
		else
		{
			setLineFlat(tx,false,durationUSec);
		}
	}

	template<typename PinT>
	inline uint32_t PulseIn(PinT p, bool lvl, uint32_t timeout, uint16_t* gap=nullptr) 
	{
		timeout = m_ts.m_timer.microsecondsToTicks(timeout);
		
		m_ts.reset();
		WallClockT ts = 0;
		WallClockT te = 0;
		m_ts.loopFast( timeout , [p,&lvl,&ts,&te](WallClockT t)
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
		else { return m_ts.m_timer.ticksToMicroseconds(te-ts); }
	}

	template<typename PinT>
	inline uint16_t RecordSignal(PinT p, uint32_t timeout, uint16_t nSamples, uint16_t* signal) 
	{
		timeout = m_ts.m_timer.microsecondsToTicks(timeout);
		WallClockT ts = 0;
		int i=0;
		bool lvl = p.Get();
		m_ts.reset();
		m_ts.loopFast( timeout , [signal,p,nSamples,&lvl,&ts,&i](WallClockT t)
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
	
	TimeSchedulerT< avrtl::AvrTimer0, WallClockT > m_ts;
};

using SignalProcessing32 = SignalProcessing<int32_t>;
using SignalProcessing16 = SignalProcessing<int16_t>;

