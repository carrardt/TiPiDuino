#ifndef __TIPIDUINO_HWSerialIO_h
#define __TIPIDUINO_HWSerialIO_h

#include <avr/io.h>
#include <avr/interrupt.h>

#include <AvrTL.h>
#include <ByteStream.h>

// Warning !! Needs Wiring lib ISRs defined as weak symblols
// ISR(vect,__attribute__ ((weak)))

struct HWSerialIO : public ByteStream
{
	static constexpr uint8_t U2X = 1;
	static constexpr uint8_t RXCIE = 7;
	static constexpr uint8_t RXEN = 4;
	static constexpr uint8_t TXEN = 3;
	static constexpr uint8_t UDRIE = 5;
	static constexpr uint8_t FLAGS_EN = (1<<TXEN) | (1<<RXEN) | (1<<RXCIE);
	static constexpr uint8_t BufferSize = 16;

	static volatile uint8_t Tx_byte;
	static volatile uint8_t Rx_byte;

	static inline void begin(uint32_t baud=9600)
	{
	  uint16_t ubrrValue;
	  uint16_t calcUBRRU2X;
	  uint16_t calcUBRR;
	  uint32_t calcBaudU2X;
	  uint32_t calcBaud;

	  // Calculate for U2X (with rounding)
	  calcUBRR = (F_CPU/2/baud + 4) / 8;
	  calcUBRRU2X = (F_CPU/baud + 4) / 8;

	  calcBaud = F_CPU/16/calcUBRR;
	  calcBaudU2X = F_CPU/8/calcUBRRU2X;

	  if ( avrtl::abs(calcBaudU2X - baud) < avrtl::abs(calcBaud - baud))
	  {
		UCSR0A = 1 << U2X;
		ubrrValue = calcUBRRU2X - 1;
	  }
	  else
	  {
		UCSR0A = 0;
		ubrrValue = calcUBRR - 1;
	  }
	  UCSR0C = 0b110;
	  UBRR0H = ubrrValue >> 8;
	  UBRR0L = ubrrValue;
	  UCSR0B = FLAGS_EN;
	}
	
	virtual bool writeByte( uint8_t x )
	{
		while( Tx_byte!=0 ) {}
		uint8_t oldSREG = SREG;
		cli();
		HWSerialIO::Tx_byte = x;
		UCSR0B |= (1 << UDRIE);
		HWSerialIO::Rx_byte = 0;
		SREG = oldSREG;
		return true;
	}

	virtual uint8_t readByte()
	{
		HWSerialIO::Rx_byte = 0;
		while( Tx_byte!=0 ) {}
		char c;
		while( ( c = HWSerialIO::Rx_byte ) == 0 ) {}
		HWSerialIO::Rx_byte = 0;
		return c;
	}

};

#endif
