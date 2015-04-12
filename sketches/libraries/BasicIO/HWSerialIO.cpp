#include "HWSerialIO.h"

volatile uint8_t HWSerialIO::Tx_byte = 0;
volatile uint8_t HWSerialIO::Rx_byte = 0;

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
	HWSerialIO::Rx_byte = UDR0;
}
