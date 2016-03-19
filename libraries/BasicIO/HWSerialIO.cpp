#include "HWSerialIO.h"

#define RX_BUF_SIZE 8;

volatile uint8_t HWSerialIO::Tx_byte = 0;
volatile uint8_t HWSerialIO::Rx_buf[RX_BUF_SIZE];
volatile uint8_t HWSerialIO::Rx_in = 0;
volatile uint8_t HWSerialIO::Rx_read = 0;

const char* HWSerialIO::endline() const { return "\n\r"; }

void HWSerialIO::begin(uint32_t baud)
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

bool HWSerialIO::writeByte( uint8_t x )
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

uint8_t HWSerialIO::readByte()
{
	while( Rx_read == Rx_in ) ;
	uint8_t c = Rw_buf[Rx_read];
	Rx_read = (Rx_read+1)%RX_BUF_SIZE;
	return c;
}

ISR(USART_UDRE_vect)
{
	uint8_t b = HWSerialIO::Tx_byte;
	if( b != 0 )
	{
		UDR0 = b;
		HWSerialIO::Tx_byte = 0;
	}
	UCSR0B = HWSerialIO::FLAGS_EN;
}

ISR(USART_RX_vect)
{
	uint8_t nextIn = (Rx_in+1)%RX_BUF_SIZE;
	if(nextIn==Rx_read) return;
	Rx_buf[Rx_in] = UDR0;
	Rx_in = nextIn;
}
