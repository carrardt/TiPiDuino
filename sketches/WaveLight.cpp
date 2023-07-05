#include <AvrTL.h>
#include <AvrTLPin.h>

#include "BasicIO/ByteStream.h"
#include "BasicIO/InputStream.h"
#include "BasicIO/PrintStream.h"

#include <PCD8544.h>

#include "WS2811/ws2811.h"

#include <avr/interrupt.h>


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

// PCD8544 pins : SCLK, SDIN, DC, RST, SCE
#define LCD_PINS     2,    3,  4,   5, PCD8544_UNASSIGNED
static PCD8544 lcd( LCD_PINS );

static ByteStreamAdapter<PCD8544> lcdIO;
static PrintStream cout;
static avrtl::AvrTimer1NoPrescaler g_hires_timer;

// A custom glyph (a smiley)...
static const uint8_t glyph[] = { 0b00010000, 0b00110100, 0b00110000, 0b00110100, 0b00010000 };

void setup()
{
	cli();

  lcdIO.m_rawIO.setPins( LCD_PINS );
  
  // PCD8544-compatible displays may have a different resolution...
  lcdIO.m_rawIO.begin(84, 48);

  // Add the smiley to position "0" of the ASCII table...
  lcdIO.m_rawIO.createChar(1, glyph);
  
    // Use a potentiometer to set the LCD contrast...
  // short level = map(analogRead(A0), 0, 1023, 0, 127);
  lcdIO.m_rawIO.setContrast(63);

  cout.begin(&lcdIO);

	ws2811_pin.SetOutput();

  g_hires_timer.start();
}

void loop()
{
  static const uint8_t test_buffer[] = { 0x05 , 0x0A , 0x05 };
  uint16_t T0 = g_hires_timer.m_timerhw.counter();
  ws2811_send_bytes_PB0( test_buffer , 3 );
  uint16_t T1 = g_hires_timer.m_timerhw.counter();
  cout << "T="<< (T1-T0) << '\n';
  avrtl::delayMicroseconds( 1000000 );
}

