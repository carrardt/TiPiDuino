#ifndef __AVRTL_PIN_H
#define __AVRTL_PIN_H

#include <stdint.h>
#include <avr/io.h>

#include "WiringUnoBoardDefs.h" // for standard pin mapping. From Wiring lib

namespace avrtl
{		
	template<uint8_t _PORT> struct StaticPinGroup { };

#	if defined(DDRD) && defined(PIND) && defined(PORTD) 
	template<> struct StaticPinGroup<0>
	{
		static void SetOutput(uint8_t mask) { DDRD |= mask; }
		static void SetInput(uint8_t mask) { DDRD &= ~mask; }
		static uint8_t Get() { return PIND; }
		// not optimal but it has a constant execution time
		static void Set(uint8_t x , uint8_t mask=0xFF)
		{
			PORTD |= mask&x;
			PORTD &= (~mask)|x;
		}
	};
	
	template<> struct StaticPinGroup<1>
#	else
	template<> struct StaticPinGroup<0>
#	endif
	{
		static void SetOutput(uint8_t mask) { DDRB |= mask; }
		static void SetInput(uint8_t mask) { DDRB &= ~mask; }
		static uint8_t Get() { return PINB; }
		static void Set(uint8_t x , uint8_t mask=0xFF)
		{
			PORTB |= mask&x;
			PORTB &= (~mask)|x;
		}
	};

#	if defined(DDRC) && defined(PINC) && defined(PORTC) 
	template<> struct StaticPinGroup<2>
	{
		static void SetOutput(uint8_t mask) { DDRC |= mask; }
		static void SetInput(uint8_t mask) { DDRC &= ~mask; }
		static uint8_t Get() { return PINC; }
		static void Set(uint8_t x , uint8_t mask=0xFF)
		{
			PORTC |= mask&x;
			PORTC &= (~mask)|x;
		}
	};
#	endif

	template<typename PinGroupT, uint8_t _pinBit>
	struct StaticPinT
	{
		static constexpr uint8_t pinbit = _pinBit;
		static constexpr uint8_t setmask = 1<<pinbit;
		static constexpr uint8_t clearmask = ~setmask;

		inline StaticPinT() : m_pg() {}
		inline StaticPinT( PinGroupT _pg ) : m_pg(_pg) {}

		inline void SetOutput() const { m_pg.SetOutput(setmask); }
		inline void SetInput() const { m_pg.SetInput(setmask); }
		inline void Set(bool b) const
		{
			uint8_t x = b; // guaranteed to be 0 or 1 only
			m_pg.Set( x<<pinbit, setmask );
		}
		inline bool Get() const
		{ 
			return m_pg.Get() & setmask;
		}
		inline operator bool() const { return Get(); }
		inline bool operator = (bool b) const { Set(b); return b; }

		const PinGroupT m_pg;
	};

	// standard definition for most Arduino alike boards
	template<uint8_t pinId> struct PinMapping {};
	template<> struct PinMapping<0> { using PinGroup = StaticPinGroup<0>; static constexpr uint8_t PinBit = 0; };
	template<> struct PinMapping<1> { using PinGroup = StaticPinGroup<0>; static constexpr uint8_t PinBit = 1; };
	template<> struct PinMapping<2> { using PinGroup = StaticPinGroup<0>; static constexpr uint8_t PinBit = 2; };
	template<> struct PinMapping<3> { using PinGroup = StaticPinGroup<0>; static constexpr uint8_t PinBit = 3; };
	template<> struct PinMapping<4> { using PinGroup = StaticPinGroup<0>; static constexpr uint8_t PinBit = 4; };
	template<> struct PinMapping<5> { using PinGroup = StaticPinGroup<0>; static constexpr uint8_t PinBit = 5; };
	template<> struct PinMapping<6> { using PinGroup = StaticPinGroup<0>; static constexpr uint8_t PinBit = 6; };
	template<> struct PinMapping<7> { using PinGroup = StaticPinGroup<0>; static constexpr uint8_t PinBit = 7; };

#	if defined(DDRD) && defined(PIND) && defined(PORTD) 
	template<> struct PinMapping<8> { using PinGroup = StaticPinGroup<1>; static constexpr uint8_t PinBit = 0; };
	template<> struct PinMapping<9> { using PinGroup = StaticPinGroup<1>; static constexpr uint8_t PinBit = 1; };
	template<> struct PinMapping<10> { using PinGroup = StaticPinGroup<1>; static constexpr uint8_t PinBit = 2; };
	template<> struct PinMapping<11> { using PinGroup = StaticPinGroup<1>; static constexpr uint8_t PinBit = 3; };
	template<> struct PinMapping<12> { using PinGroup = StaticPinGroup<1>; static constexpr uint8_t PinBit = 4; };
	template<> struct PinMapping<13> { using PinGroup = StaticPinGroup<1>; static constexpr uint8_t PinBit = 5; };
#	endif

#	if defined(DDRC) && defined(PINC) && defined(PORTC) 
	template<> struct PinMapping<14> { using PinGroup = StaticPinGroup<2>; static constexpr uint8_t PinBit = 0; };
	template<> struct PinMapping<15> { using PinGroup = StaticPinGroup<2>; static constexpr uint8_t PinBit = 1; };
	template<> struct PinMapping<16> { using PinGroup = StaticPinGroup<2>; static constexpr uint8_t PinBit = 2; };
	template<> struct PinMapping<17> { using PinGroup = StaticPinGroup<2>; static constexpr uint8_t PinBit = 3; };
	template<> struct PinMapping<18> { using PinGroup = StaticPinGroup<2>; static constexpr uint8_t PinBit = 4; };
	template<> struct PinMapping<19> { using PinGroup = StaticPinGroup<2>; static constexpr uint8_t PinBit = 5; };
#	endif
	
	// TODO: change StaticPin name to something like 'ArduinoStaticPin'
	template<uint8_t p>
	using StaticPin = StaticPinT< typename PinMapping<p>::PinGroup, PinMapping<p>::PinBit >;

	template<typename PinGroupT, uint8_t _b1, uint8_t _b2>
	struct StaticDualPinT
	{
		static constexpr uint8_t Pin1Bit = _b1;
		static constexpr uint8_t Pin2Bit = _b2;
		static constexpr uint8_t Pin1Mask = 1 << Pin1Bit;
		static constexpr uint8_t Pin2Mask = 1 << Pin2Bit;
		static constexpr uint8_t AllPinMask = Pin1Mask | Pin2Mask;
		
		inline StaticDualPinT() : m_mask(AllPinMask) {}
		inline StaticDualPinT(PinGroupT pg) : m_pg(pg) , m_mask(AllPinMask) {}
		void SelectPin(bool pselect) { m_mask = pselect ? Pin2Mask : Pin1Mask ; }
		void SelectAllPins() { m_mask = AllPinMask; }
		void SetOutput() const { m_pg.SetOutput(AllPinMask); }
		void SetInput() const { m_pg.SetInput(AllPinMask); }
		void Set(bool b) const { m_pg.Set( b ? AllPinMask : 0 , m_mask ); }
		bool Get() const { return m_pg.Get() & m_mask ; }
		operator bool() const { return Get(); }
		bool operator = (bool b) const { Set(b); return b; }

		uint8_t m_mask;
		PinGroupT m_pg;
	};

	// TODO: change the type name
	// TODO: check if pin group of p1 and p2 are the same
	template<uint8_t p1,uint8_t p2>
	using DualPin = StaticDualPinT< typename PinMapping<p1>::PinGroup
								  , PinMapping<p1>::PinBit
								  , PinMapping<p2>::PinBit > ;

	// not const pointers to volatile, just pointers to volatile.
	// make the entire object const if needed
	struct DynamicPinGroup
	{
		inline DynamicPinGroup(volatile uint8_t * ddr, volatile uint8_t * pin, volatile uint8_t * port)
			: m_ddr(ddr)
			, m_pin(pin)
			, m_port(port)
			{}

		void SetOutput(uint8_t mask) const { *m_ddr |= mask; }
		void SetInput(uint8_t mask) const { *m_ddr &= ~mask; }
		uint8_t Get() const { return *m_pin; }
		void Set(uint8_t x , uint8_t mask=0xFF) const
		{
			*m_port |= mask&x;
			*m_port &= (~mask)|x;
		}

		volatile uint8_t * m_ddr;
		volatile uint8_t * m_pin;
		volatile uint8_t * m_port;
	};

	template<typename PinGroupT>
	struct DynamicPinT
	{
		inline DynamicPinT() : m_mask(0) {}
		inline DynamicPinT( PinGroupT pg ) : m_pg(pg), m_mask(0) {}
		inline DynamicPinT( PinGroupT pg, uint8_t pinBit ) : m_pg(pg), m_mask(1<<pinBit) {}
		inline void SetOutput() const { m_pg.SetOutput(m_mask); }
		inline void SetInput() const { m_pg.SetInput(m_mask); }
		inline void Set(bool b) const { m_pg.Set( b ? m_mask : 0 , m_mask ); }
		inline bool Get() const	{ return m_pg.Get() & m_mask; }
		inline operator bool() const { return Get(); }
		inline bool operator = (bool b) const { Set(b); return b; }
		PinGroupT m_pg;
		uint8_t m_mask;
	};

	inline DynamicPinT<DynamicPinGroup> make_pin(uint8_t p)
	{
		uint8_t g = WdigitalPinToPort(p);
		DynamicPinGroup pg( WportModeRegister(g), WportInputRegister(g), WportOutputRegister(g) );
		return DynamicPinT<DynamicPinGroup>( pg, WdigitalPinToBit(p) );
	}

	struct NullPin
	{
		static void SetOutput() {}
		static void SetInput() {}
		static void Set(bool b) {}
		static constexpr bool Get() { return false; }
		operator bool() const { return Get(); }
		bool operator = (bool b) const { Set(b); return b; }
	};
	
}

// compatibility layer
#if !defined(ARDUINO_MAIN) && !defined(Arduino_h)
inline void pinMode(uint8_t pId, uint8_t mode)
{
	auto p = avrtl::make_pin(pId);
	if( mode == INPUT ) p.SetInput();
	else if( mode == OUTPUT ) p.SetOutput();
	else if( mode == INPUT_PULLUP ) { p.SetInput(); p.Set(HIGH); }
}

inline void digitalWrite(uint8_t pId, bool level)
{
	avrtl::make_pin(pId).Set( level );
}

inline bool digitalRead(uint8_t pId)
{
	return avrtl::make_pin(pId).Get();
}
#endif

#endif
