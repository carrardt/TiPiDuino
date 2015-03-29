/*
 * AvrTL.h
 * 
 * Copyright 2015  <picuntu@g8picuntu>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
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

namespace avrtl
{
	static constexpr uint32_t TIMER_CPU_RATIO = TIMER0PRESCALEFACTOR / (F_CPU / 1000000UL);

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
	static void blink(LedPinT& led, int N=10)
	{
		led = true;
		for(int j=0;j<N;j++)
		{
			led = j&1;
			DelayMicroseconds(100000);
		}
	}
template<uint32_t speed> struct BaudRate { };

template< int _pinId >
struct AvrPin
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

	static uint32_t PulseIn(bool lvl, uint32_t timeout) 
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
		while( ts<timeout && Get()!=lvl ) { UPDATE_CLOCK_COUNTER(); ts=CLOCK_ELAPSED(); }
		if( ts >= timeout ) { SREG=oldSREG; return 0; }
		
		UPDATE_CLOCK_COUNTER();
		uint32_t te = CLOCK_ELAPSED();
		while( (te-ts)<timeout && Get()==lvl ) { UPDATE_CLOCK_COUNTER(); te=CLOCK_ELAPSED(); }
		
		SREG=oldSREG;
		if( (te-ts) >= timeout ) return 0;
		
		return (te-ts) * TIMER_CPU_RATIO;

#undef UPDATE_CLOCK_COUNTER
#undef CLOCK_ELAPSED
	}

	template< uint32_t speed >
	static void serialWriteByte(uint8_t b, BaudRate<speed>)
	{
	  static constexpr uint32_t bitDelayTicks = 1000000UL / ( speed * TIMER_CPU_RATIO );
	  Set( false );
	  DelayTimerTicks(bitDelayTicks);
	  for (uint8_t mask=0x01;mask;mask<<=1) 
	  {
		Set( (b&mask)!=0  );
		DelayTimerTicks(bitDelayTicks);
	  }
	  Set( true );
	  DelayTimerTicks(bitDelayTicks);
	}

	template< uint32_t speed >
	static inline void serialBegin(BaudRate<speed>)
	{
	  static constexpr uint32_t bitDelayTicks = 1000000UL / ( speed * TIMER_CPU_RATIO );
	  Set( true );
	  DelayTimerTicks(bitDelayTicks);
	}

    operator bool() const { return Get(); }
    bool operator = (bool b) const { Set(b); return b; }

#undef pin_addr
#undef port_addr
#undef ddr_addr
#undef pin_bit
};

template<int P>
static constexpr AvrPin<P> make_pin() { return AvrPin<P>(); }

#define pin(P) make_pin<P>()

}



// some wiring compatibility tricks
extern void loop();
extern void setup();
int main(void) __attribute__ ((noreturn,OS_main,weak));
int main(void)
{
	avrtl::boardInit();
	setup();
	for(;;) loop();
}

#endif
