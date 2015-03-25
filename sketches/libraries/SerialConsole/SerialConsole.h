#ifndef __TIPIDUINO_SerialConsole_h
#define __TIPIDUINO_SerialConsole_h

#include <avr/io.h>
#include <avr/interrupt.h>
#include <AvrTL.h>

// Warning !! Needs Wirinlib ISRs defined as weak symblols
// ISR(vect,__attribute__ ((weak)))

static volatile uint8_t __SerialConsole_byte = 0;

struct SerialConsole
{
	static constexpr uint8_t U2X = 1;
	static constexpr uint8_t TXEN = 3;
	static constexpr uint8_t UDRIE = 5;

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

	  if (abs(calcBaudU2X - baud) < abs(calcBaud - baud))
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
	  UCSR0B = 1 << TXEN;
	}
	
	static inline void writeChar( char x )
	{
		while( __SerialConsole_byte!=0 ) {}
		uint8_t oldSREG = SREG;
		cli();
		__SerialConsole_byte = x;
		UCSR0B |= (1 << UDRIE);
		SREG = oldSREG;
	}
	
	SerialConsole operator << ( char x ) const
	{
		writeChar( x );
		return SerialConsole();
	}
	
	SerialConsole operator << ( const char* s ) const
	{
		while(*s != '\0') writeChar( *(s++) );
		return SerialConsole();
	}
	
	static void print(unsigned long x, int base=10, int ndigits=0) { print((long)x,base,ndigits); }
	static void print(unsigned int x, int base=10, int ndigits=0) { print((long)x,base,ndigits); }
	static void print(int x, int base=10, int ndigits=0) { print((long)x,base,ndigits); }
	static void print(long x, int div=10, int ndigits=0)
	{
		if(div<2) return;
		char digits[16];
		int n = 0;
		if( x < 0 ) { writeChar((char)'-'); x=-x; }
		do
		{
			digits[n++] = x % div;
			x /= div;
		} while( x > 0 );
		for(int i=0;i<(ndigits-n);i++) writeChar('0');
		for(int i=0;i<n;i++)
		{
			int dg = digits[n-i-1];
			writeChar ( (dg<10) ? (char)('0'+dg) : (char)('A'+(dg-10)) );
		}
	}
};

ISR(USART_UDRE_vect)
{
	uint8_t b = __SerialConsole_byte;
	if( b != 0 )
	{
		UDR0 = b;
		__SerialConsole_byte = 0;
	}
	UCSR0B = 1 << SerialConsole::TXEN ; 
}

#endif
