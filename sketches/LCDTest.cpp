#include <AvrTL.h>
#include <PrintStream.h>
#include <LCD.h>

using namespace avrtl;

#define LED_PIN 13

//constexpr auto led = pin(&PINB,&PORTB,&DDRB,5);
/*
constexpr auto led = pin( portInputRegister(digitalPinToPort(LED_PIN))
						, portOutputRegister(digitalPinToPort(LED_PIN))
						, portModeRegister(digitalPinToPort(LED_PIN))
						,digitalPinToBit(LED_PIN) );
*/
static constexpr auto led = pin(LED_PIN);
#define LCD_PINS 7,6,5,4,3,2 // respectively RS, EN, D7, D6, D5, D4
LCD<LCD_PINS> lcd;
PrintStream< LCD<LCD_PINS> > cout(lcd);

void setup()
{
	led.SetOutput(); //pinMode(LED_PIN,OUTPUT);
	dbgOutput.begin(9600);
	lcd.begin();
}

void loop()
{
	cout<<"Hello World ;)\n";
	for(int j=0;j<5;j++)
	{
		for(int i=0;i<6;i++)
		{
		led = (i%2==0);
		DelayMicroseconds(500000UL*j);
		}
	}
}
