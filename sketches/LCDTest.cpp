#include <AvrTL.h>
#include <LCD1602.h>
#include <BasicIO/PrintStream.h>

/*
 * LCD1602	Arduino
 * VSS		GND
 * VDD		+5v
 * VO		Pontentiometer(10K)/resistor
 * RS		D7
 * RW		GND
 * EN		D6
 * D4		D5
 * D5		D4
 * D6		D3
 * D7		D2
 * A		+5v
 * K		GND
 */

using namespace avrtl;

#define LCD_PINS 7,6,5,4,3,2 // respectively RS, EN, D7, D6, D5, D4
LCD1602<LCD_PINS> lcd;
PrintStream cout;

void setup()
{
	lcd.begin();
	cout.begin( &lcd );
}

static int COUNTER=0;
void loop()
{
	cout << "Counter=" << COUNTER << '\n';
	++COUNTER;
	DelayMicroseconds(5000000UL);
}

