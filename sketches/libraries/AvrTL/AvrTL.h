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

#include <Wiring.h>
#include <stdint.h>

namespace avrtl
{

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
    
    const AvrPin& operator = (bool b) const { Set(b); return *this; }

private:
	const PIp pin_addr;
	const POp port_addr;
	const DDp ddr_addr;
	const uint8_t pin_bit;
};

#define AUTO_RET(x...) -> decltype((x)) { return (x); }

template<typename PinT, typename PortT, typename DdrT>
constexpr auto pin(PinT* pi, PortT* po, DdrT* dd, uint8_t b) AUTO_RET( AvrPin<decltype(pi),decltype(po),decltype(dd)>(pi,po,dd,b) )

constexpr auto pin(int pinId)
	AUTO_RET(
		pin( portInputRegister(digitalPinToPort(pinId))
		   , portOutputRegister(digitalPinToPort(pinId))
		   , portModeRegister(digitalPinToPort(pinId))
		   , digitalPinToBit(pinId) )
		)

}

#endif
