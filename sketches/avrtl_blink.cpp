#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <PrintStream.h>
#include <HWSerialIO.h>

using namespace avrtl;

#define LED_PIN 13

static constexpr auto led = StaticPin<LED_PIN>();
HWSerialIO hwserial;
PrintStream cout;

void setup()
{
	led.SetOutput(); //pinMode(LED_PIN,OUTPUT);
	hwserial.begin(9600);
	cout.begin(&hwserial);
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
