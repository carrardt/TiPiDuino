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
static bool state = false;

void setup()
{
	led.SetOutput(); //pinMode(LED_PIN,OUTPUT);
}

void loop()
{
	state = !state;
	led = state; //digitalWrite( LED_PIN, state ? HIGH : LOW );
	delay(500);
}
