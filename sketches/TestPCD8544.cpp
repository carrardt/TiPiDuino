/*
 * PCD8544 - Interface with Philips PCD8544 (or compatible) LCDs.
 *
 * Copyright (c) 2010 Carlos Rodrigues <cefrodrigues@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * To use this sketch, connect the eight pins from your LCD like thus:
 *
 * Pin 1 -> +3.3V (rightmost, when facing the display head-on)
 * Pin 2 -> Arduino digital pin 3
 * Pin 3 -> Arduino digital pin 4
 * Pin 4 -> Arduino digital pin 5
 * Pin 5 -> Arduino digital pin 7
 * Pin 6 -> Ground
 * Pin 7 -> 10uF capacitor -> Ground
 * Pin 8 -> Arduino digital pin 6
 *
 * Since these LCDs are +3.3V devices, you have to add extra components to
 * connect it to the digital pins of the Arduino (not necessary if you are
 * using a 3.3V variant of the Arduino, such as Sparkfun's Arduino Pro).
 */

#include <PCD8544.h>
#include "BasicIO/PrintStream.h"
#include "AvrTL/timer.h"

/*
 * Important Note : if you connect screen's SCE pin to ground, it just works !
 * it seems SCE pin prevents undesired messages to be received.
 */

// PCD8544 pins :         SCLK, SDIN, DC, RST, SCE
#define LCD_PINS_ATTINY85    1,    2,  3,   4, PCD8544_UNASSIGNED
#define LCD_PINS_ATTINY84    5,    4,  3,   1, 2
#define LCD_PINS_ATMEGA328   8,    7,  6,   5, PCD8544_UNASSIGNED

#if defined(__AVR_ATtiny84__)
#define LCD_PINS LCD_PINS_ATTINY84
static avrtl::NullPin led;
#elif defined(__AVR_ATtiny85__)
#define LCD_PINS LCD_PINS_ATTINY85
static avrtl::NullPin led;
#elif defined(__AVR_ATmega328__)
#define LCD_PINS LCD_PINS_ATMEGA328
static auto led = avrtl::StaticPin<13>();
#else
#error MCU not supported
#endif

// A custom glyph (a smiley)...
static const uint8_t glyph[] = { 0b00010000, 0b00110100, 0b00110000, 0b00110100, 0b00010000 };

static ByteStreamAdapter<PCD8544> lcdIO;
static PrintStream cout;

void setup()
{
  lcdIO.m_rawIO.setPins( LCD_PINS );
  
  // PCD8544-compatible displays may have a different resolution...
  lcdIO.m_rawIO.begin(84, 48);

  // Add the smiley to position "0" of the ASCII table...
  lcdIO.m_rawIO.createChar(1, glyph);
  
    // Use a potentiometer to set the LCD contrast...
  // short level = map(analogRead(A0), 0, 1023, 0, 127);
  lcdIO.m_rawIO.setContrast(63);

  cout.begin(&lcdIO);
}


void loop()
{
  // Just to show the program is alive...
  static int counter = 0;
  static bool ledState = false;

  // Write a piece of text on the first line...
  lcdIO.m_rawIO.setCursor(0, 0);
  cout << "Hello, World!";

  // Write the counter on the second line...
  lcdIO.m_rawIO.setCursor(0, 1);
  cout << counter << ' ' << '\1';

  led = ledState;
  ledState = ! ledState;

  avrtl::delay(950);
  counter++;
}


