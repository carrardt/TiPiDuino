#ifndef __TIPIDUINO_SoftSerialIO_h
#define __TIPIDUINO_SoftSerialIO_h

#include <stdint.h>
#include <ByteStream.h>
#include <AvrTL.h>
//#include <DigitalOutput.h>

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
		/*
	  TimingBufferSignal<10,false> serialByteSignal(false);
	  serialByteSignal.timings[0] = bitDelay;
	  int j=0;
	  bool prevLevel = false;
	  for(int i=0;i<8;++i)
	  {
		  bool nextLevel = ( (b&0x01) != 0 );
		  if( nextLevel == prevLevel )
		  {
			  serialByteSignal.timings[j] += bitDelay;
		  }
		  else
		  {
			  ++j;
			  serialByteSignal.timings[j] = bitDelay;
			  prevLevel = !prevLevel;
		  }
	  }
	  if(prevLevel) { serialByteSignal.timings[j] += bitDelay*2; }
	  else { ++j; serialByteSignal.timings[j] = bitDelay*2; }
	  for(;j<10;++j) { serialByteSignal.timings[j]=0; }
	  digitalOutput( bitDelay*10, serialByteSignal, tx );
	  */

	  byte mask;
	  uint8_t oldSREG = SREG;
	  cli();
	  
	  tx = 0;
	  avrtl::DelayMicroseconds(bitDelay);
	  for (mask = 0x01; mask; mask <<= 1) 
	  {
		if (b & mask) tx = 1;
		else tx = 0;
		avrtl::DelayMicroseconds(bitDelay);
	  }
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
