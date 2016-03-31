#ifndef __TiDuino_TimeScheduler_h
#define __TiDuino_TimeScheduler_h

#include <AvrTLSignal.h>

struct TimeScheduler
{
	int16_t m_wallclock;
	uint8_t m_t;
	avrtl::SignalProcessingMode m_signalMode;
	
	inline void start()
	{
		m_signalMode.start();
		m_wallclock = 0;
		m_t = TCNT0;
	}

	inline void stop()
	{
		m_signalMode.stop();
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

#endif
