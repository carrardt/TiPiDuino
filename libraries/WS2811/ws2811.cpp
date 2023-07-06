#include <AvrTL.h>
#include <AvrTLPin.h>
#include <timer.h>

/*
 * WS2811 signal structure
 *          __
 * bit 0 : |   |________|  : High = 0.5 µs , Low = 2 µs , total= 2.5 µs
 *          _______
 * bit 1 : |       |____|  : High = 1.2 µs , Low = 1.3 µs , toal = 2.5 µs
 *
 * if we split the 2 signals in 3 phase, we have :
 * 0.5 µs High , 0.7 µs High/Low depending on bit to encode (Low for 0, High for 1) , 1.3 µs Low
 *
 * It means we have to write 1, then the bit to encode and finaly 0 to output port.
 * In between writes we have a little time to perform simple operations (i.e. fetch next bit)
 *
 * possible LEDs input current setup : R = (Vs - Vled) / Iled
 * Test model is 10W aRGB, input is DC12V , datasheet says I=300mA for R,G,B , U=6-6.6v for RED , U=9-9.6v for GREEN and BLUE
 * Rr = (12-6.3)/0.3 = 19 Ohm
 * Rg = Rb = (12-9.3)/0.3 = 9 Ohm
 */

avrtl::StaticPin<8> ws2811_pin(); // pin 8 => PB0 on atmega328p

void ws2811_send_bytes_PB0( const uint8_t* s, uint16_t len)
{
  const uint8_t zero = PORTB & 0xFE ;
  const uint8_t one =  zero | 0x01;
  const uint8_t* end = s+len;
  uint8_t byte = 0;
  uint8_t bit = 0;

  PORTB = zero;

  //avrtl::delayMicroseconds( 50 );

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

