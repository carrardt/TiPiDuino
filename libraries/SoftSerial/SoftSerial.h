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
	static constexpr uint32_t bitDelay( uint32_t i )
	{
		return ( ((i+1)*1000000UL) / BaudRate ) - ( (i*1000000UL) / BaudRate );
	}
	static constexpr uint32_t bitPeriod = 1000000UL / BaudRate;

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
	  
	  // avoid detecting a start bit while still reading last byte's ending 0
	  while( ! rx.Get() ) ;
	  // wait for start bit
	  while( rx.Get() ) ;
	  
	  ts.reset();
	  ts.exec( bitDelay(0) + bitPeriod/4, [](){} );
	  // read bits
	  ts.exec( bitDelay(1), [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );
	  ts.exec( bitDelay(2), [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );
	  ts.exec( bitDelay(3), [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );
	  ts.exec( bitDelay(4), [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );
	  ts.exec( bitDelay(5), [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );
	  ts.exec( bitDelay(6), [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );
	  ts.exec( bitDelay(7), [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );
	  ts.exec( bitDelay(8), [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );
	  ts.stop();
	  return C;
	}

	inline bool writeByte(uint8_t b)
	{
	  ts.start();
	  ts.exec( 64, [](){} ); // just to absorb startup time
	  ts.exec( bitDelay(0), [this](){ tx = LOW; } );
	  ts.exec( bitDelay(1), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.exec( bitDelay(2), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.exec( bitDelay(3), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.exec( bitDelay(4), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.exec( bitDelay(5), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.exec( bitDelay(6), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.exec( bitDelay(7), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.exec( bitDelay(8), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.exec( bitDelay(9), [this](){ tx = HIGH; } );
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
