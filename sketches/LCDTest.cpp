#include <AvrTL.h>
#include <LCD1602.h>
#include <BasicIO/PrintStream.h>

/*
 * LCD1602	Arduino
 * VSS		GND
 * VDD		+5v
 * VO		Pontentiometer(10K)/resistor
 * RS		D2
 * RW		GND
 * EN		D3
 * D4		D9
 * D5		D10
 * D6		D11
 * D7		D12
 * A		+5v
 * K		GND
 */
#define LCD_PINS 2,3,9,10,11,12 // respectively RS, EN, D4, D5, D6, D7

using namespace avrtl;

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

