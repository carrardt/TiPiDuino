#include <AvrTL.h>
#include <AvrTLPin.h>
#include <timer.h>

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

