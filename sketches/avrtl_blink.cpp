#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTLSignal.h>

using namespace avrtl;

#define LED_PIN1 11
#define LED_PIN2 12
#define LED_PIN3 13

static constexpr auto led1 = StaticPin<LED_PIN1>();
static constexpr auto led2 = StaticPin<LED_PIN2>();
static constexpr auto led3 = StaticPin<LED_PIN3>();

void setup()
{
	led1.SetOutput(); //pinMode(LED_PIN,OUTPUT);
	led2.SetOutput(); //pinMode(LED_PIN,OUTPUT);
	led3.SetOutput(); //pinMode(LED_PIN,OUTPUT);
}

void loop()
{
	for(int j=0;j<10;j++)
	{
		for(int i=0;i<10;i++)
		{
			led1 = true;
			DelayMicroseconds(100000UL*j);
			led1 = false;
			led2 = true;
			DelayMicroseconds(100000UL*j);
			led2 = false;
			led3 = true;
			DelayMicroseconds(100000UL*j);
			led1 = false;
			led2 = false;
			led3 = false;
		}
	}
}

