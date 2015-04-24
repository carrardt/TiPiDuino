#include <AvrTL.h>
#include <LCD.h>
#include <PrintStream.h>

static auto led = avrtl::StaticPin<13>();
#define LCD_PINS 7,6,5,4,3,2 // respectively RS, EN, D7, D6, D5, D4
LCD<LCD_PINS> lcd;
PrintStream cout;

void setup()
{
	Serial.begin(9600);
	lcd.begin();
	cout.begin(&lcd);
}

void loop()
{
	if( Serial.available() )
	{
		led=1;
		cout << (char) Serial.read();
		led=0;
	}
}
