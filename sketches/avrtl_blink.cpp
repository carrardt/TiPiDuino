#include <Wiring.h>
#include <AvrTL.h>

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

void setup()
{
	led.SetOutput(); //pinMode(LED_PIN,OUTPUT);
}

void loop()
{
	for(int j=0;j<5;j++)
	{
		for(int i=0;i<6;i++)
		{
		led = (i%2==0); //digitalWrite( LED_PIN, state ? HIGH : LOW );
		delay(500*j);
		}
	}
}
