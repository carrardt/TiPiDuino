#ifndef __TIPIDUINO_SoftSerialIO_h
#define __TIPIDUINO_SoftSerialIO_h

#include <stdint.h>
#include <ByteStream.h>
#include <AvrTL.h>

template<typename _RxPinT, typename _TxPinT, uint32_t _BaudRate>
struct SoftSerialIO : public ByteStream
{
	using RxPin = _RxPinT;
	using TxPin = _TxPinT;
	static constexpr uint32_t BaudRate = _BaudRate;
	static constexpr uint32_t bitPeriod = 1000000UL / BaudRate;
	static constexpr uint32_t bitDelay = bitPeriod-1; // Works fine up to 19200 bauds
	
	RxPin rx;
	TxPin tx;
	
	SoftSerialIO(const RxPin& _rx, const TxPin& _tx) : rx(_rx), tx(_tx) {}
	
	void begin()
	{
		rx.SetInput();
		tx.SetOutput();
		//digitalWrite(transmitPin, HIGH);
		tx = 1;
		avrtl::DelayMicroseconds( bitPeriod ); // if we were low this establishes the end
	}

	virtual bool writeByte(uint8_t b)
	{
	  byte mask;
	  
	  uint8_t oldSREG = SREG;
	  cli();
	  
	  tx = 0;
	  //digitalWrite(transmitPin, LOW);
	  avrtl::DelayMicroseconds(bitDelay);

	  for (mask = 0x01; mask; mask <<= 1) 
	  {
		if (b & mask) // choose bit
		{ 
		  //digitalWrite(transmitPin,HIGH); // send 1
		  tx = 1;
		}
		else
		{
		  // digitalWrite(transmitPin,LOW); // send 0
		  tx = 0;
		}
		avrtl::DelayMicroseconds(bitDelay);
	  }

	  //digitalWrite(transmitPin, HIGH);
	  tx = 1;
	  avrtl::DelayMicroseconds(bitDelay);

	  SREG=oldSREG;
	  
	  return true;
	}

};

template<uint32_t br, typename _RxPinT, typename _TxPinT>
static SoftSerialIO<_RxPinT,_TxPinT,br> make_softserial(const _RxPinT& rx, const _TxPinT& tx)
{
	return  SoftSerialIO<_RxPinT,_TxPinT,br>(rx,tx);
}

#endif
