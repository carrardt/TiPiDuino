#include <Arduino.h>

#include <AvrTL.h>
#include <LCD1602.h>
#include <BasicIO/PrintStream.h>

static auto led = avrtl::StaticPin<13>();
LCD1602<7,6,5,4,3,2> lcd; // respectively RS, EN, D7, D6, D5, D4
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
