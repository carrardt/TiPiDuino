#ifndef __TIPIDUINO_DigitalOutput_h
#define __TIPIDUINO_DigitalOutput_h

#include <AvrTL.h>

struct FlatSignal
{
	inline uint32_t toggleTime() const { return 0; }
	inline void toggle() const { }
	inline operator bool() const { return false; }
};

template<uint16_t _Size, bool _Loop=true>
struct TimingBufferSignal
{
	inline TimingBufferSignal(bool init_lvl=false) : i(0), level(init_lvl) {}
	inline void toggle()
	{
		if( _Loop && i==_Size ) i=0;
		if( i < _Size )
		{
			level=!level;
			++i;
		}
	}
	inline uint32_t toggleTime() const
	{
		return timings[i];
	}
	inline operator bool() const { return level; }

	uint32_t timings[_Size];
	uint16_t i;
	bool level;
};

// combinaison de 2 signaux en un classe
template<typename SignalT, typename PinT>
static void digitalOutput(uint32_t duration, SignalT& signal, PinT& wire )
{
#define UPDATE_CLOCK_COUNTER() do{ t2=TCNT0; if(t2<t)++m; t=t2; }while(0)
#define CLOCK_ELAPSED() ((m*256+t)-t0)

	duration /= TIMER_CPU_RATIO;
	uint8_t oldSREG = SREG;
	cli();
	uint32_t m = 0;
	uint8_t t = TCNT0, t0=t, t2;
	
	wire = signal;
	uint32_t nextToggleTime = CLOCK_ELAPSED() + signal.toggleTime()/TIMER_CPU_RATIO;
	
	do
	{
		UPDATE_CLOCK_COUNTER();
		uint32_t elapsed = CLOCK_ELAPSED();
		while( elapsed<nextToggleTime ) { UPDATE_CLOCK_COUNTER(); elapsed=CLOCK_ELAPSED(); }
		signal.toggle();
		wire = signal;
		nextToggleTime += signal.toggleTime()/TIMER_CPU_RATIO;
		UPDATE_CLOCK_COUNTER();
	}
	while( CLOCK_ELAPSED() < duration );
	
	SREG = oldSREG;

#undef UPDATE_CLOCK_COUNTER
#undef CLOCK_ELAPSED
}


#endif
