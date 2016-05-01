#ifndef __TIDUINO_SoftSerial_h
#define __TIDUINO_SoftSerial_h

#include <stdint.h>
#include <TimeScheduler/TimeScheduler.h>
#include <AvrTL/WiringUnoBoardDefs.h>

template<typename _RxPinT, typename _TxPinT, uint32_t _BaudRate = 9600>
struct SoftSerialIO 
{
	using RxPin = _RxPinT;
	using TxPin = _TxPinT;
	static constexpr uint32_t BaudRate = _BaudRate;
	static constexpr uint32_t bitPeriod = 1000000UL / BaudRate;
	static constexpr uint32_t timeShift = bitPeriod + bitPeriod/4;
	static constexpr uint32_t byteGap = bitPeriod/4;

	TimeScheduler ts;
	RxPin rx;
	TxPin tx;

	SoftSerialIO(RxPin _rx, TxPin _tx) : rx(_rx), tx(_tx) {}

	void begin()
	{
		rx.SetInput();
		tx.SetOutput();
		tx = HIGH;
	}

	inline uint8_t readByte()
	{
	  uint8_t C = 0;
	  ts.start();
	  while( rx.Get() ) ts.reset();
	  ts.exec( timeShift, [](){} );
	  ts.exec( bitPeriod, [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );	  
	  ts.exec( bitPeriod, [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );	  
	  ts.exec( bitPeriod, [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );	  
	  ts.exec( bitPeriod, [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );	  
	  ts.exec( bitPeriod, [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );	  
	  ts.exec( bitPeriod, [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );	  
	  ts.exec( bitPeriod, [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );	  
	  ts.exec( bitPeriod, [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );
	  ts.exec( byteGap, [](){} );
	  ts.stop();
	  return C;
	}
	
	inline bool writeByte(uint8_t b)
	{
	  ts.start();
	  ts.exec( 64, [](){} ); // just to absorb startup time
	  ts.exec( bitPeriod, [this](){ tx = LOW; } );
	  ts.exec( bitPeriod, [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.exec( bitPeriod, [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.exec( bitPeriod, [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.exec( bitPeriod, [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.exec( bitPeriod, [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.exec( bitPeriod, [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.exec( bitPeriod, [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.exec( bitPeriod, [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.exec( bitPeriod, [this](){ tx = HIGH; } );
	  ts.stop();
	  return true;
	}

};

template<uint32_t br, typename _RxPinT, typename _TxPinT>
static SoftSerialIO<_RxPinT,_TxPinT,br> make_softserial(_RxPinT rx, _TxPinT tx)
{
	return  SoftSerialIO<_RxPinT,_TxPinT,br>(rx,tx);
}

#endif
