#include <AvrTL.h>
#include <AvrTLPin.h>

#include "BasicIO/ByteStream.h"
#include "BasicIO/InputStream.h"
#include "BasicIO/PrintStream.h"

#include <PCD8544.h>

#include "WS2811/ws2811.h"

#include <avr/interrupt.h>

// PCD8544 pins : SCLK, SDIN, DC, RST, SCE (connect to ground)
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

  lcdIO.m_rawIO.setCursor(0, 0);
  cout << "Hello, World!";

	ws2811_pin.SetOutput();

  g_hires_timer.start();
}

void loop()
{
  static int counter = 0;

  uint8_t tmp[6] = { 0xFF , 0xFF , 0xFF , 0xF0 , 0xF0 , 0xF0 };
  ++ counter;
  
  //uint16_t T0 = g_hires_timer.m_timerhw.counter();
  ws2811_send_bytes_PB0( tmp , 6 );
  avrtl::delayMicroseconds( 150 );

  //uint16_t T1 = g_hires_timer.m_timerhw.counter();

  //lcdIO.m_rawIO.setCursor(0, 1);
  //cout << "T="<< (T1-T0) << ' ' << '\1' << '\n';
}

