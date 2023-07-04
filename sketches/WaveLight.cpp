#include <AvrTL.h>
#include <AvrTLPin.h>

#include "BasicIO/PrintStream.h"
#include "BasicIO/InputStream.h"

#include <avr/interrupt.h>

#define SERIAL_SPEED 19200
#include "HWSerialNoInt/HWSerialNoInt.h"

/*
 * WS2811 signal structure
 *          __
 * bit 0 :    |________ : High = 0.5 µs , Low = 2 µs , total= 2.5 µs
 *         _____
 * bit 1 :      |______ : High = 1.2 µs , Low = 1.3 µs , toal = 2.5 µs
 *
 * if we split the 2 signals in 3 phase, we have :
 * 0.5 µs High , 0.7 µs High/Low depending on bit to encode (Low for 0, High for 1) , 1.3 µs Low
 *
 * It means we have to write 1, then the bit to encode and finaly 0 to output port.
 * In between writes we have a little time to perform simple operations (i.e. fetch next bit)
 *
 */

ByteStreamAdapter<HWSerialNoInt,10000UL> serialIO;
PrintStream cout;
avrtl::AvrTimer0NoPrescaler g_hires_timer;
auto ws2811_pin = avrtl::StaticPin<8>(); // pin 8 => PB0 on atmega328p

void setup()
{
	cli();
	serialIO.m_rawIO.begin(SERIAL_SPEED);
	cout.begin( &serialIO );
	ws2811_pin.SetOutput();
}

void ws2811_send_bytes_PB0( const uint8_t* s, uint16_t len)
{
  const uint8_t zero = PORTB & 0xFE ;
  const uint8_t one =  zero | 0x01;
  const uint8_t* end = buffer+len;
  uint8_t byte = 0;
  uint8_t bit = 0;

  PORTB = zero;

  delayMicroseconds( 50 );

  do
  {
    // bit 0
    PORTB = one;
    bit = zero | ( byte & 0x01 );
    // wait to reach 0.5 µs period
    PORTB = bit;
    byte = byte >> 1;
    // wait to reach 0.7 µs period
    PORTB = zero;
    // wait to reach 1.3 µs period

    // bit 1
    PORTB = one;
    bit = zero | ( byte & 0x01 );
    // wait to reach 0.5 µs period
    PORTB = bit;
    byte = byte >> 1;
    // wait to reach 0.7 µs period
    PORTB = zero;
    // wait to reach 1.3 µs period

    // bit 2
    PORTB = one;
    bit = zero | ( byte & 0x01 );
    // wait to reach 0.5 µs period
    PORTB = bit;
    byte = byte >> 1;
    // wait to reach 0.7 µs period
    PORTB = zero;
    // wait to reach 1.3 µs period

    // bit 3
    PORTB = one;
    bit = zero | ( byte & 0x01 );
    // wait to reach 0.5 µs period
    PORTB = bit;
    byte = byte >> 1;
    // wait to reach 0.7 µs period
    PORTB = zero;
    // wait to reach 1.3 µs period

    // bit 4
    PORTB = one;
    bit = zero | ( byte & 0x01 );
    // wait to reach 0.5 µs period
    PORTB = bit;
    byte = byte >> 1;
    // wait to reach 0.7 µs period
    PORTB = zero;
    // wait to reach 1.3 µs period

    // bit 5
    PORTB = one;
    bit = zero | ( byte & 0x01 );
    // wait to reach 0.5 µs period
    PORTB = bit;
    byte = byte >> 1;
    // wait to reach 0.7 µs period
    PORTB = zero;
    // wait to reach 1.3 µs period

    // bit 6
    PORTB = one;
    bit = zero | ( byte & 0x01 );
    // wait to reach 0.5 µs period
    PORTB = bit;
    byte = byte >> 1;
    // wait to reach 0.7 µs period
    PORTB = zero;
    // wait to reach 1.3 µs period

    // bit 7
    PORTB = one;
    bit = zero | ( byte & 0x01 );
    // wait to reach 0.5 µs period
    PORTB = bit;
    // wait to reach 0.7 µs period
    PORTB = zero;
    byte = *s++ ;
    // wait to reach 1.3 µs period
  } while( s != end );


}


void loop()
{
  ws2811_pin.Set( false );
  auto T0 = g_hires_timer.m_timerhw.counter();
  auto T1 = g_hires_timer.m_timerhw.counter();
  ws2811_pin.Set( true );
  auto T2 = g_hires_timer.m_timerhw.counter();

	cout << n << ", E="<<(T1-T0)<<", S="<<(T2-T1)<< '\n';
}


