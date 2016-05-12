#ifndef __TIDUINO_FastSerial_h
#define __TIDUINO_FastSerial_h

#include <stdint.h>
#include <AvrTL/WiringUnoBoardDefs.h>

template<typename _RxPinT, typename _TxPinT>
struct FastSerial
{
	using RxPin = _RxPinT;
	using TxPin = _TxPinT;

	FastSerial(RxPin _rx, TxPin _tx) : m_rx(_rx), m_tx(_tx) {}

	inline void begin()
	{
		m_rx.SetOutput(); // debug
		//m_rx.SetInput(); 
		m_tx.SetOutput();
		m_tx = HIGH;
	}

	inline void writeBit(bool b)
	{
		m_tx.Set(LOW);
		m_tx.Set(HIGH);
		if( b ) { m_tx.Set(HIGH); m_tx.Set(HIGH); }
	}
	inline void stopBit()
	{
		m_tx.Set(LOW);
		m_tx.Set(HIGH);
	}
	inline void startBit()
	{
		for(uint8_t i=0;i<4;i++) m_tx.Set(LOW);
		for(uint8_t i=0;i<4;i++) m_tx.Set(HIGH);
	}

	template<uint8_t NBits=32>
	inline void write(uint32_t word)
	{
		m_rx = HIGH; // debug
		startBit();
		for(uint8_t i=0;i<NBits;i++)
		{
			writeBit( word & 0x01 );
			word >>= 1;
		}
		stopBit();
		m_rx = LOW; // debug
	}

	template<uint8_t NBits=32>
	inline uint32_t read()
	{
		uint8_t buf[NBits];
		uint8_t l=0;
		do
		{
			l = 0;
			while( m_rx.Get() );
			while( !m_rx.Get() ) ++l;
		} while( l<10 );
		while( m_rx.Get() );
		for(int i=0;i<NBits;i++)
		{
			buf[i] = 0;
			while( !m_rx.Get() );
			while( m_rx.Get() ) ++buf[i];
		}
		uint32_t n = 0;
		for(int i=0;i<NBits;i++)
		{
			uint32_t b = (buf[i]>5) ? (1UL<<i) : 0 ;
			n |= b;
		}
		return n;
	}

	RxPin m_rx;
	TxPin m_tx;
};

template<typename _RxPinT, typename _TxPinT>
static FastSerial<_RxPinT,_TxPinT> make_fastserial(_RxPinT rx, _TxPinT tx)
{
	return FastSerial<_RxPinT,_TxPinT>(rx,tx);
}


#endif
