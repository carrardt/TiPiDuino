#ifndef __TIDUINO_SoftSerial_h
#define __TIDUINO_SoftSerial_h

#include <stdint.h>
#include <TimeScheduler/TimeScheduler.h>
#include <AvrTL/WiringUnoBoardDefs.h>

template<typename _RxPinT, typename _TxPinT, uint32_t _BaudRate = 9600, typename _SchedulerT=TimeScheduler >
struct SoftSerialIO 
{
	using RxPin = _RxPinT;
	using TxPin = _TxPinT;
	using SchedulerT = _SchedulerT;
	using TimerT = typename SchedulerT::BaseTimerT;
	
	static constexpr uint32_t BaudRate = _BaudRate;
	static constexpr uint32_t bitDelay( uint32_t i )
	{
		return ( ((i+1)*TimerT::TicksPerSecond) / BaudRate ) - ( (i*TimerT::TicksPerSecond) / BaudRate );
	}
	static constexpr uint32_t bitPeriod = TimerT::TicksPerSecond / BaudRate;
	static constexpr uint32_t timeShift = (BaudRate<=38400) ? (bitPeriod/4) : ( (BaudRate<=57600) ? (bitPeriod/8) : 0 );
	
	SchedulerT ts;
	RxPin rx;
	TxPin tx;

	SoftSerialIO(RxPin _rx, TxPin _tx) : rx(_rx), tx(_tx) {}

	inline void begin()
	{
		rx.SetInput();
		tx.SetOutput();
		tx = HIGH;
	}

	inline void begin(int /*baud*/)
	{
	  begin();
	  ts.start();
	}

	inline uint8_t readByte()
	{
		ts.start();
		uint8_t C = readByteFast();
		ts.stop();
		return C;
	}
	
	inline uint8_t readByteFast()
	{
	  uint8_t C = 0;
	  
	  // avoid detecting a start bit while still reading last byte's ending 0
	  while( ! rx.Get() ) ;
	  // wait for start bit
	  while( rx.Get() ) ;
	  
	  ts.reset();
	  ts.execFast( bitDelay(0) + timeShift, [](){} );
	  // read bits
	  ts.execFast( bitDelay(1), [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );
	  ts.execFast( bitDelay(2), [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );
	  ts.execFast( bitDelay(3), [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );
	  ts.execFast( bitDelay(4), [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );
	  ts.execFast( bitDelay(5), [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );
	  ts.execFast( bitDelay(6), [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );
	  ts.execFast( bitDelay(7), [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );
	  ts.execFast( bitDelay(8), [this,&C](){ uint8_t bit=rx.Get(); C=(C>>1)|(bit<<7); } );
	  
	  return C;
	}

	inline bool writeByte(uint8_t b)
	{
	  ts.start();
	  ts.execFast( 64000/TimerT::NanoSecPerTick, [](){} ); // just to absorb startup time
	  ts.execFast( bitDelay(0), [this](){ tx = LOW; } );
	  ts.execFast( bitDelay(1), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.execFast( bitDelay(2), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.execFast( bitDelay(3), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.execFast( bitDelay(4), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.execFast( bitDelay(5), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.execFast( bitDelay(6), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.execFast( bitDelay(7), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.execFast( bitDelay(8), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.execFast( bitDelay(9), [this](){ tx = HIGH; } );
	  ts.stop();
	  return true;
	}

	inline bool writeByteFast(uint8_t b)
	{
	  ts.execFast( 64000/TimerT::NanoSecPerTick, [](){} ); // just to absorb startup time
	  ts.execFast( bitDelay(0), [this](){ tx = LOW; } );
	  ts.execFast( bitDelay(1), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.execFast( bitDelay(2), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.execFast( bitDelay(3), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.execFast( bitDelay(4), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.execFast( bitDelay(5), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.execFast( bitDelay(6), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.execFast( bitDelay(7), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.execFast( bitDelay(8), [this,&b](){ tx = b&0x01; b>>=1; } );
	  ts.execFast( bitDelay(9), [this](){ tx = HIGH; } );
	  return true;
	}

	inline bool writeBufferFast(uint8_t* s)
	{
	  uint8_t b = 0;
	  while( ( b=(*(s++)) ) != '\0' ) { writeByteFast(b); }
	  return true;
	}

};

template<uint32_t br, typename _RxPinT, typename _TxPinT>
static SoftSerialIO<_RxPinT,_TxPinT,br> make_softserial(_RxPinT rx, _TxPinT tx)
{
	return  SoftSerialIO<_RxPinT,_TxPinT,br>(rx,tx);
}

template<uint32_t br, typename SchedulerT, typename _RxPinT, typename _TxPinT>
static SoftSerialIO<_RxPinT,_TxPinT,br,SchedulerT> make_softserial_hr(_RxPinT rx, _TxPinT tx)
{
	return  SoftSerialIO<_RxPinT,_TxPinT,br,SchedulerT>(rx,tx);
}

#endif
