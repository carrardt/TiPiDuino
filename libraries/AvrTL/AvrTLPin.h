#ifndef __AVRTL_PIN_H
#define __AVRTL_PIN_H

#include <stdint.h>
#include <avr/io.h>
#include "WiringUnoBoardDefs.h" // for standard pin mapping. From Wiring lib

namespace avrtl
{

#	define DECLARE_PIN_GROUP(G) 								\
	struct PinGroup##G { 									\
		static void SetOutput(uint8_t mask) { DDR##G |= mask; } \
		static void SetInput(uint8_t mask) { DDR##G &= ~mask; } \
		static uint8_t Get() { return PIN##G ; } 				\
		static void Set(uint8_t x , uint8_t mask=0xFF) 			\
		{														\
			PORT##G |= mask&x;									\
			PORT##G &= (~mask)|x; 								\
		} }

#	if defined(DDRA) && defined(PINA) && defined(PORTA)
		DECLARE_PIN_GROUP(A);
	#endif

#	if defined(DDRB) && defined(PINB) && defined(PORTB)
		DECLARE_PIN_GROUP(B);
	#endif

#	if defined(DDRC) && defined(PINC) && defined(PORTC)
		DECLARE_PIN_GROUP(C);
	#endif

#	if defined(DDRD) && defined(PIND) && defined(PORTD)
		DECLARE_PIN_GROUP(D);
	#endif

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

	template<uint8_t pinId> struct PinMapping {};
	
#	ifdef __AVR_ATtiny84__
	template<> struct PinMapping<0> { using PinGroup = PinGroupA; static constexpr uint8_t PinBit = 0; };
	template<> struct PinMapping<1> { using PinGroup = PinGroupA; static constexpr uint8_t PinBit = 1; };
	template<> struct PinMapping<2> { using PinGroup = PinGroupA; static constexpr uint8_t PinBit = 2; };
	template<> struct PinMapping<3> { using PinGroup = PinGroupA; static constexpr uint8_t PinBit = 3; };
	template<> struct PinMapping<4> { using PinGroup = PinGroupA; static constexpr uint8_t PinBit = 4; };
	template<> struct PinMapping<5> { using PinGroup = PinGroupA; static constexpr uint8_t PinBit = 5; };
	template<> struct PinMapping<6> { using PinGroup = PinGroupA; static constexpr uint8_t PinBit = 6; };
	template<> struct PinMapping<7> { using PinGroup = PinGroupA; static constexpr uint8_t PinBit = 7; };

	template<> struct PinMapping<8> { using PinGroup = PinGroupB; static constexpr uint8_t PinBit = 2; };
	template<> struct PinMapping<9> { using PinGroup = PinGroupB; static constexpr uint8_t PinBit = 1; };
	template<> struct PinMapping<10> { using PinGroup = PinGroupB; static constexpr uint8_t PinBit = 0; };
#	endif

#	ifdef __AVR_ATtiny85__
	template<> struct PinMapping<0> { using PinGroup = PinGroupB; static constexpr uint8_t PinBit = 0; };
	template<> struct PinMapping<1> { using PinGroup = PinGroupB; static constexpr uint8_t PinBit = 1; };
	template<> struct PinMapping<2> { using PinGroup = PinGroupB; static constexpr uint8_t PinBit = 2; };
	template<> struct PinMapping<3> { using PinGroup = PinGroupB; static constexpr uint8_t PinBit = 3; };
	template<> struct PinMapping<4> { using PinGroup = PinGroupB; static constexpr uint8_t PinBit = 4; };
#	endif

	// standard definition for most Arduino UNO alike boards
#	ifdef __AVR_ATmega328P__
	template<> struct PinMapping<0> { using PinGroup = PinGroupD; static constexpr uint8_t PinBit = 0; };
	template<> struct PinMapping<1> { using PinGroup = PinGroupD; static constexpr uint8_t PinBit = 1; };
	template<> struct PinMapping<2> { using PinGroup = PinGroupD; static constexpr uint8_t PinBit = 2; };
	template<> struct PinMapping<3> { using PinGroup = PinGroupD; static constexpr uint8_t PinBit = 3; };
	template<> struct PinMapping<4> { using PinGroup = PinGroupD; static constexpr uint8_t PinBit = 4; };
	template<> struct PinMapping<5> { using PinGroup = PinGroupD; static constexpr uint8_t PinBit = 5; };
	template<> struct PinMapping<6> { using PinGroup = PinGroupD; static constexpr uint8_t PinBit = 6; };
	template<> struct PinMapping<7> { using PinGroup = PinGroupD; static constexpr uint8_t PinBit = 7; };
	
	template<> struct PinMapping<8> { using PinGroup = PinGroupB; static constexpr uint8_t PinBit = 0; };
	template<> struct PinMapping<9> { using PinGroup = PinGroupB; static constexpr uint8_t PinBit = 1; };
	template<> struct PinMapping<10> { using PinGroup = PinGroupB; static constexpr uint8_t PinBit = 2; };
	template<> struct PinMapping<11> { using PinGroup = PinGroupB; static constexpr uint8_t PinBit = 3; };
	template<> struct PinMapping<12> { using PinGroup = PinGroupB; static constexpr uint8_t PinBit = 4; };
	template<> struct PinMapping<13> { using PinGroup = PinGroupB; static constexpr uint8_t PinBit = 5; };

	template<> struct PinMapping<14> { using PinGroup = PinGroupC; static constexpr uint8_t PinBit = 0; };
	template<> struct PinMapping<15> { using PinGroup = PinGroupC; static constexpr uint8_t PinBit = 1; };
	template<> struct PinMapping<16> { using PinGroup = PinGroupC; static constexpr uint8_t PinBit = 2; };
	template<> struct PinMapping<17> { using PinGroup = PinGroupC; static constexpr uint8_t PinBit = 3; };
	template<> struct PinMapping<18> { using PinGroup = PinGroupC; static constexpr uint8_t PinBit = 4; };
	template<> struct PinMapping<19> { using PinGroup = PinGroupC; static constexpr uint8_t PinBit = 5; };

# define SDA 18
# define SCL 19

#	endif // __AVR_ATmega328P__

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
		[[maybe_unused]] const uint8_t g = WdigitalPinToPort(p);
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
void pinMode(uint8_t pId, uint8_t mode);
void digitalWrite(uint8_t pId, bool level);
bool digitalRead(uint8_t pId);
uint8_t shiftIn(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder);
void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
#endif

#endif
