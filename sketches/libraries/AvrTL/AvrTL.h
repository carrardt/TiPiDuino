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

#include <Wiring.h> 
/*#ifndef NOT_A_PORT
#define NOT_A_PORT 0xFF
#endif*/
#include <BoardDefs.h> // for pin mapping

namespace avrtl
{
	static void DelayMicroseconds(uint32_t timeout)
	{
		timeout /= TIMER0PRESCALEFACTOR / (F_CPU / 1000000UL);
		uint8_t oldSREG = SREG;
		cli();
		
		uint32_t m=0;
		uint8_t t=TCNT0, t0=t;
		
#define UPDATE_CLOCK_COUNTER() do{ uint8_t t2=TCNT0; if(t2<t) ++m; t=t2; }while(0)
#define CLOCK_ELAPSED() ((m*256+t)-t0)

		UPDATE_CLOCK_COUNTER();
		while( CLOCK_ELAPSED() < timeout ) { UPDATE_CLOCK_COUNTER(); }

#undef UPDATE_CLOCK_COUNTER
#undef CLOCK_ELAPSED

		SREG = oldSREG;
	}

	template<typename LedPinT>
	static void blink(LedPinT& led)
	{
		for(int j=0;j<100;j++)
		{
			led = j&1;
			DelayMicroseconds(100000);
		}
	}

template< int _pinId >
struct AvrPin
{
#define pin_addr (portInputRegister(digitalPinToPort(_pinId)))
#define port_addr (portOutputRegister(digitalPinToPort(_pinId)))
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
		timeout /= TIMER0PRESCALEFACTOR / (F_CPU / 1000000UL);
		
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
		
		return (te-ts) * ( TIMER0PRESCALEFACTOR / (F_CPU / 1000000UL) );

#undef UPDATE_CLOCK_COUNTER
#undef CLOCK_ELAPSED
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

#endif
