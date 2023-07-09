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

// A custom glyph (a smiley)...
static const uint8_t glyph[] = { 0b00010000, 0b00110100, 0b00110000, 0b00110100, 0b00010000 };

static Adafruit_NeoPixel strip(16);

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

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
}

void loop()
{  
  static int counter = 0;
  
  avrtl::delayMicroseconds( 100000 );

  for(int i=0;i<16;i++)
  {
    strip.setPixelColor(i , counter%255 , (counter+128)%255 , (counter+64)%255 );
  }
  strip.show();

  ++ counter;
  lcdIO.m_rawIO.setCursor(0, 0);
  cout << "STEP=" << counter;
}

