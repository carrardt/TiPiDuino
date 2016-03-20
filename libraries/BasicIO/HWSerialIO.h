#ifndef __TIPIDUINO_HWSerialIO_h
#define __TIPIDUINO_HWSerialIO_h

#include <avr/io.h>
#include <avr/interrupt.h>

#include <AvrTL.h>
#include <BasicIO/ByteStream.h>

// Warning !! Needs Wiring lib ISRs defined as weak symblols
// ISR(vect,__attribute__ ((weak)))

struct HWSerialIO : public ByteStream
{
	static void begin(uint32_t baud=9600);
	virtual const char* endline() const;
	virtual bool writeByte( uint8_t x );
	virtual uint8_t readByte();
	virtual int16_t available();
	
	static constexpr uint8_t U2X = 1;
	static constexpr uint8_t RXCIE = 7;
	static constexpr uint8_t RXEN = 4;
	static constexpr uint8_t TXEN = 3;
	static constexpr uint8_t UDRIE = 5;
	static constexpr uint8_t FLAGS_EN = (1<<TXEN) | (1<<RXEN) | (1<<RXCIE);
	static constexpr uint8_t BufferSize = 2;

	static volatile uint8_t Tx_byte;
	static volatile bool Tx_ready;
	static volatile uint8_t Rx_read;
	static volatile uint8_t Rx_avail;
	static volatile uint8_t Rx_buf[BufferSize];
};

#endif
