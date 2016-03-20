#include <AvrTL.h>
#include <LCD1602.h>
#include <BasicIO/PrintStream.h>

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

