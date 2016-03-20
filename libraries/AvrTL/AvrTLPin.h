#ifndef __AVRTL_PIN_H
#define __AVRTL_PIN_H

#include <stdint.h>
#include <avr/io.h>

#include "WiringUnoBoardDefs.h" // for standard pin mapping. From Wiring lib

namespace avrtl
{
	template<int _p1, int _p2, bool SamePort = (WdigitalPinToPort(_p1)==WdigitalPinToPort(_p2)) >
	struct DualPin {};

	template<int _p1, int _p2>
	struct DualPin<_p1,_p2,true>
	{
		#define pin_addr (WportInputRegister(WdigitalPinToPort(_p1)))
		#define port_addr (WdigitalPinToPortReg(_p1))
		#define ddr_addr (WportModeRegister(WdigitalPinToPort(_p1)))
		#define pin1_bit (WdigitalPinToBit(_p1))
		#define pin2_bit (WdigitalPinToBit(_p2))

		uint8_t mask;

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
		uint8_t ClearAllMask() const { return ~SetAllMask(); }
		
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
		operator bool() const { return Get(); }
		bool operator = (bool b) const { Set(b); return b; }
		
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
		operator bool() const { return Get(); }
		bool operator = (bool b) const { Set(b); return b; }
	};

	template< int _pinId >
	struct StaticPin
	{
		#define pin_addr (WportInputRegister(WdigitalPinToPort(_pinId)))
		#define port_addr (WdigitalPinToPortReg(_pinId))
		#define ddr_addr (WportModeRegister(WdigitalPinToPort(_pinId)))
		#define pin_bit (WdigitalPinToBit(_pinId))
		
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
		operator bool() const { return Get(); }
		bool operator = (bool b) const { Set(b); return b; }
		
		#undef pin_addr
		#undef port_addr
		#undef ddr_addr
		#undef pin_bit
	};

}

#endif
