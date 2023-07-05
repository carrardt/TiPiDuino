#include <AvrTL.h>
#include <AvrTLPin.h>

#include "BasicIO/PrintStream.h"
#include "BasicIO/InputStream.h"

#include "WS2811/ws2811.h"

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
 * possible LEDs input current setup : R = (Vs - Vled) / Iled
 * Test model is 10W aRGB, input is DC12V , datasheet says I=300mA for R,G,B , U=6-6.6v for RED , U=9-9.6v for GREEN and BLUE
 * Rr = (12-6.3)/0.3 = 19 Ohm
 * Rg = Rb = (12-9.3)/0.3 = 9 Ohm
 */

ByteStreamAdapter<HWSerialNoInt,10000UL> serialIO;
PrintStream cout;
avrtl::AvrTimer0NoPrescaler g_hires_timer;

void setup()
{
	cli();
	serialIO.m_rawIO.begin(SERIAL_SPEED);
	cout.begin( &serialIO );

	ws2811_pin.SetOutput();
}

void loop()
{
  static const uint8_t test_buffer[] = { 0x05 , 0x0A , 0x05 };
  ws2811_send_bytes_PB0( test_buffer , 3 );
}

