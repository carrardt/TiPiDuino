#include "HWSerialIO.h"

volatile uint8_t HWSerialIO::Tx_byte = 0;
volatile bool HWSerialIO::Tx_ready = true;
volatile uint8_t HWSerialIO::Rx_buf[HWSerialIO::BufferSize];
volatile uint8_t HWSerialIO::Rx_read = 0;
volatile uint8_t HWSerialIO::Rx_avail = 0;

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
	while( ! Tx_ready );
	Tx_ready = false;
	HWSerialIO::Tx_byte = x;
	uint8_t oldSREG = SREG;
	cli();
	UCSR0B |= (1 << UDRIE);
	SREG = oldSREG;
	return true;
}

int16_t HWSerialIO::available()
{
	return HWSerialIO::Rx_avail;
}

uint8_t HWSerialIO::readByte()
{
	while( HWSerialIO::Rx_avail == 0 ) ;
	uint8_t c = Rx_buf[Rx_read];
	Rx_read = (Rx_read+1) % BufferSize;
	-- HWSerialIO::Rx_avail;
	return c;
}

ISR(USART_UDRE_vect)
{
	UDR0 = HWSerialIO::Tx_byte;
	UCSR0B = HWSerialIO::FLAGS_EN;
	HWSerialIO::Tx_ready = true;
}

ISR(USART_RX_vect)
{
	// on buffer overflow, throw away incoming bytes
	uint8_t data = UDR0;
	if(HWSerialIO::Rx_avail<HWSerialIO::BufferSize)
	{
		uint8_t toWrite = ( HWSerialIO::Rx_read + HWSerialIO::Rx_avail ) % HWSerialIO::BufferSize;
		HWSerialIO::Rx_buf[toWrite] = data;
		++ HWSerialIO::Rx_avail;
	}
}
