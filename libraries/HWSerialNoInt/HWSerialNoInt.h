#ifndef __TIDUINO_HWSerialNoInt_h
#define __TIDUINO_HWSerialNoInt_h

#include <avr/io.h>

struct HWSerialNoInt
{
	static constexpr uint8_t U2X = 1;

	static int32_t abs(int32_t x)
	{
		return (x<0) ? -x : x;
	}

	static void begin(int32_t baud)
	{
	  uint16_t ubrrValue;
	  uint16_t calcUBRRU2X;
	  uint16_t calcUBRR;
	  int32_t calcBaudU2X;
	  int32_t calcBaud;

	  // Calculate for U2X (with rounding)
	  calcUBRR = (F_CPU/2/baud + 4) / 8;
	  calcUBRRU2X = (F_CPU/baud + 4) / 8;

	  calcBaud = F_CPU/16/calcUBRR;
	  calcBaudU2X = F_CPU/8/calcUBRRU2X;

	  if ( HWSerialNoInt::abs(calcBaudU2X - baud) < HWSerialNoInt::abs(calcBaud - baud))
	  {
		UCSR0A = 1 << U2X;
		ubrrValue = calcUBRRU2X - 1;
	  }
	  else
	  {
		UCSR0A = 0;
		ubrrValue = calcUBRR - 1;
	  }
	  UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);
	  UBRR0H = ubrrValue >> 8;
	  UBRR0L = ubrrValue;
	  UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	}

	static bool writeByte( uint8_t u8Data )
	{
		//wait while previous byte is completed
		while(!(UCSR0A&(1<<UDRE0))) {};
		// Transmit data
		UDR0 = u8Data;
		return true;
	}

	static uint16_t readByteAsync()
	{
		uint16_t x = 0;
		if( UCSR0A&(1<<RXC0) )
		{
			x = 256;
			x |= UDR0;
		}
		return x;
	}

	static uint8_t readByte()
	{
		// Wait for byte to be received
		while(!(UCSR0A&(1<<RXC0))){};
		// Return received data
		return UDR0;
	}

};

#endif
