/*
 * AvrTL.h
 * 
 * No Copyright
 * 
 */

#ifndef __AvrTL_H
#define __AvrTL_H

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <stdlib.h>

#define NOT_A_PORT 0xFF
#define NOT_A_REG  NULL
// #define LOW false
// #define HIGH true
#include <BoardDefs.h> // for pin mapping

#ifndef BUILD_TIMESTAMP
#define BUILD_TIMESTAMP 0
#endif

namespace avrtl
{
	// from timer ticks to microseconds
	static constexpr uint32_t TIMER_CPU_RATIO = TIMER0PRESCALEFACTOR / (F_CPU / 1000000UL);

	// from cpu clock ticks / microseconds
	static constexpr uint32_t MSEC_CPU_RATIO = (F_CPU / 1000000UL);

	static inline void boardInit()
	{
		// Sets the timer prescale factor to 64;
		TCCR0B = (TCCR0B & 0b11111000) | 0b011;

		// start interrupts
		sei();
	}

	template<typename T>
	static inline T abs(T x) { return (x<0) ? (-x) : x ; }

	static uint8_t checksum8(const uint8_t* buf, int nbytes)
	{
		uint8_t cs = 0;
		for(int i=0;i<nbytes;i++)
		{
			cs = ( (cs<<1) | (cs >>7) ) ^ buf[i];
		}
		return cs;
	}

	static void eeprom_gently_write_byte(uint8_t* ptr, uint8_t value)
	{
		if( eeprom_read_byte(ptr) != value )
		{
			eeprom_write_byte(ptr,value);
		}
	}

	static void eeprom_gently_write_block(const uint8_t* src, uint8_t* ptr, uint16_t N)
	{
		for(;N;--N) eeprom_gently_write_byte(ptr++,*(src++));
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
		DelayTimerTicks( timeout / TIMER_CPU_RATIO );
	}
	
	template<typename LedPinT>
	static void blink(LedPinT& led, int N=20)
	{
		for(int j=1;j<=N;j++)
		{
			led = j&1;
			DelayMicroseconds(100000);
		}
	}
	
	template<typename T>
	static void rotateBufferLeft1(T * ptr, int n)
	{
		T first = ptr[0];
		for(int i=0;i<(n-1);i++)
		{
			ptr[i] = ptr[i+1];
		}
		ptr[n-1] = first;
	}

	template<typename T>
	static void rotateBufferLeft(T * ptr, int n, int disp)
	{
		for(int i=0;i<disp;i++)
		{
			rotateBufferLeft1(ptr,n);
		}
	}
	
	template<typename T>
	static int findOccurence(const T* pattern, int psize, const T* buf, int bsize)
	{
		int j = 0;
		int start = 0;
		for(int i=0;i<bsize;i++)
		{
			if( buf[i]==pattern[j] )
			{
				++j;
				if( j == psize ) return i-psize+1;
			}
			else
			{
				i = start++;
				j = 0;
			}
		}
		return -1;
	}

template<uint32_t speed> struct BaudRate { };

template<int _p1, int _p2, bool SamePort = (digitalPinToPort(_p1)==digitalPinToPort(_p2)) >
struct DualPin {};

template<int _p1, int _p2>
struct DualPin<_p1,_p2,true>
{
#define pin_addr (portInputRegister(digitalPinToPort(_p1)))
#define port_addr (digitalPinToPortReg(_p1))
#define ddr_addr (portModeRegister(digitalPinToPort(_p1)))
#define pin1_bit (digitalPinToBit(_p1))
#define pin2_bit (digitalPinToBit(_p2))

	void SelectPin(bool pselect)
	{
		mask = 1 << ( pselect ? pin2_bit : pin1_bit ) ;
	}
	
	// equivalent to a or of input pins, and duplicate write to all pins
	void SelectAllPins()
	{
		mask = SetAllMask();
	}
	
	uint8_t SetMask() const { return mask; }
	uint8_t ClearMask() const { return ~mask; }
	
	uint8_t SetAllMask() const { return (1<<pin1_bit) | (1<<pin2_bit); }
	uint8_t ClearAllMask() const { return ~mask; }
	
    void SetOutput() const { *ddr_addr |= SetAllMask(); }
    void SetInput() const { *ddr_addr &= ClearAllMask(); }
    
    void Set(bool b) const
    {
		if(b) { *port_addr |= SetMask(); }
		else { *port_addr &= ClearMask(); }
	}
    bool Get() const
    { 
		return ( (*pin_addr) & mask ) != 0;
	}
	uint8_t mask;
#undef pin_addr
#undef port_addr
#undef ddr_addr
#undef pin_bit
};

struct NullPin
{
    static void SetOutput() {}
    static void SetInput() {}
    static void Set(bool b) {}
    static constexpr bool Get() { return false; }
};

template< int _pinId >
struct StaticPin
{
#define pin_addr (portInputRegister(digitalPinToPort(_pinId)))
#define port_addr (digitalPinToPortReg(_pinId))
#define ddr_addr (portModeRegister(digitalPinToPort(_pinId)))
#define pin_bit (digitalPinToBit(_pinId))
	static uint8_t SetMask()  { return 1<<pin_bit; }
	static uint8_t ClearMask()  { return ~SetMask(); }
    static void SetOutput()  { *ddr_addr |= SetMask(); }
    static void SetInput()  { *ddr_addr &= ClearMask(); }
    static void Set(bool b) 
    {
		if(b) { *port_addr |= SetMask(); }
		else { *port_addr &= ClearMask(); }
	}
    static bool Get() 
    { 
		return ( (*pin_addr) >> pin_bit ) & 1;
	}
#undef pin_addr
#undef port_addr
#undef ddr_addr
#undef pin_bit
};

template< typename BasePinT >
struct AvrPin : public BasePinT
{
	inline uint32_t PulseIn(bool lvl, uint32_t timeout, uint16_t* gap=0) 
	{
		timeout /= TIMER_CPU_RATIO;

		uint8_t oldSREG = SREG;
		cli();
		
		uint32_t m=0;
		uint8_t t=TCNT0, t0=t;
		
#define UPDATE_CLOCK_COUNTER() do{ uint8_t t2=TCNT0; if(t2<t) ++m; t=t2; }while(0)
#define CLOCK_ELAPSED() ((m*256+t)-t0)

		UPDATE_CLOCK_COUNTER();
		uint32_t ts = CLOCK_ELAPSED();
		while( ts<timeout && BasePinT::Get()!=lvl ) { UPDATE_CLOCK_COUNTER(); ts=CLOCK_ELAPSED(); }
		if( ts >= timeout ) { SREG=oldSREG; return 0; }
		
		UPDATE_CLOCK_COUNTER();
		uint32_t te = CLOCK_ELAPSED();
		while( (te-ts)<timeout && BasePinT::Get()==lvl ) { UPDATE_CLOCK_COUNTER(); te=CLOCK_ELAPSED(); }
		
		SREG=oldSREG;
		
		if( gap != 0 ) { *gap = ts; }
		if( (te-ts) >= timeout ) { return 0; }
		else { return (te-ts) * TIMER_CPU_RATIO; }

#undef UPDATE_CLOCK_COUNTER
#undef CLOCK_ELAPSED
	}

    operator bool() const { return BasePinT::Get(); }
    bool operator = (bool b) const { BasePinT::Set(b); return b; }
};

template<int P>
static constexpr AvrPin< StaticPin<P> > make_pin() { return AvrPin< StaticPin<P> >(); }

}

#define pin(P) AvrPin< StaticPin<P> >()


// some wiring compatibility tricks
extern void loop();
extern void setup();
int main(void) __attribute__ ((noreturn,OS_main,weak));

#endif
