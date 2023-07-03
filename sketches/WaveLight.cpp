#include <AvrTL.h>
#include <AvrTLPin.h>

#include "BasicIO/PrintStream.h"
#include "BasicIO/InputStream.h"

#include <avr/interrupt.h>

#define SERIAL_SPEED 19200
#include "HWSerialNoInt/HWSerialNoInt.h"

ByteStreamAdapter<HWSerialNoInt,10000UL> serialIO;
PrintStream cout;
int n = 0;

avrtl::AvrTimer0NoPrescaler g_hires_timer;

auto ws2811_pin = avrtl::StaticPin<9>();

void setup()
{
	cli();
	serialIO.m_rawIO.begin(SERIAL_SPEED);
	cout.begin( &serialIO );
}

void loop()
{
  ws2811_pin.Set( false );
  auto T0 = g_hires_timer.m_timerhw.counter();
  auto T1 = g_hires_timer.m_timerhw.counter();
  ws2811_pin.Set( true );
  auto T2 = g_hires_timer.m_timerhw.counter();

	n = ( n + 1 ) % 100;
	cout << n << ", E="<<(T1-T0)<<", S="<<(T2-T1)<< '\n';
}


