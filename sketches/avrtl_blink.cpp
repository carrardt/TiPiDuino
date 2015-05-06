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
	SCOPED_SIGNAL_PROCESSING;
	for(int j=0;j<10;j++)
	{
		for(int i=0;i<10;i++)
		{
			for(int k=0;k<100;k++) pulsePWMFast<100,25>(led1,(uint16_t)(1000*j));
			led1 = false;
			for(int k=0;k<100;k++) pulsePWMFast<100,25>(led2,(uint16_t)(1000*j));
			led2 = false;
			for(int k=0;k<100;k++) pulsePWMFast<100,25>(led3,(uint16_t)(1000*j));
			led3 = false;
		}
	}
}

