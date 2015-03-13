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

#include <Wiring.h> // only for NOT_A_PORT define
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


template<
	typename PIp = uint8_t volatile*,
	typename POp = uint8_t volatile*,
	typename DDp = uint8_t volatile* >
struct AvrPin
{
	constexpr AvrPin(PIp pi, POp po, DDp dd, uint8_t b)
		: pin_addr(pi)
		, port_addr(po)
		, ddr_addr(dd)
		, pin_bit(b) {}
	//Pin(PORTAddrT pa, DDRAddrT da, uint8_t b) : port_addr(pa), ddr_addr(da), pin_bit(b) {}

	uint8_t SetMask() const { return 1<<pin_bit; }
	uint8_t ClearMask() const { return ~SetMask(); }

    void SetOutput() const { *ddr_addr |= SetMask(); }
    
    void SetInput() const { *ddr_addr &= ClearMask(); }

    void Set(bool b) const
    {
		if(b) { *port_addr |= SetMask(); }
		else { *port_addr &= ClearMask(); }
	}
	
    bool Get() const
    { 
		return ( (*pin_addr) >> pin_bit ) & 1;
	}

    operator bool() const { return Get(); }
    
    bool operator = (bool b) const { Set(b); return b; }

	uint32_t PulseIn(bool lvl, uint32_t timeout) const
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
		while( te<timeout && Get()==lvl ) { UPDATE_CLOCK_COUNTER(); te=CLOCK_ELAPSED(); }
		
		SREG=oldSREG;
		if( te >= timeout ) return 0;
		return (te-ts) * ( TIMER0PRESCALEFACTOR / (F_CPU / 1000000UL) );

#undef UPDATE_CLOCK_COUNTER
#undef CLOCK_ELAPSED
	}

private:
	const PIp pin_addr;
	const POp port_addr;
	const DDp ddr_addr;
	const uint8_t pin_bit;
};

#define AVRTL_AUTO_RET(x...) -> decltype((x)) { return (x); }

template<typename PinT, typename PortT, typename DdrT>
constexpr auto pin(PinT* pi, PortT* po, DdrT* dd, uint8_t b) AVRTL_AUTO_RET( AvrPin<decltype(pi),decltype(po),decltype(dd)>(pi,po,dd,b) )

constexpr auto pin(int pinId)
	AVRTL_AUTO_RET(
		pin( portInputRegister(digitalPinToPort(pinId))
		   , portOutputRegister(digitalPinToPort(pinId))
		   , portModeRegister(digitalPinToPort(pinId))
		   , digitalPinToBit(pinId) )
		)

}

#endif
