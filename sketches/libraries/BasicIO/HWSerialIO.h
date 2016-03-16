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
	static void begin(uint32_t baud=9600);
	virtual const char* endline() const;
	virtual bool writeByte( uint8_t x );
	virtual uint8_t readByte();
	
	static constexpr uint8_t U2X = 1;
	static constexpr uint8_t RXCIE = 7;
	static constexpr uint8_t RXEN = 4;
	static constexpr uint8_t TXEN = 3;
	static constexpr uint8_t UDRIE = 5;
	static constexpr uint8_t FLAGS_EN = (1<<TXEN) | (1<<RXEN) | (1<<RXCIE);
	static constexpr uint8_t BufferSize = 16;

	static volatile uint8_t Tx_byte;
	static volatile uint8_t Rx_buf[];
};

#endif
