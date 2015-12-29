#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <PrintStream.h>
#include <HWSerialIO.h>

using namespace avrtl;

#define LED_PIN 13
#define IR_PIN 11

//constexpr auto led = pin(&PINB,&PORTB,&DDRB,5);
/*
constexpr auto led = pin( portInputRegister(digitalPinToPort(LED_PIN))
						, portOutputRegister(digitalPinToPort(LED_PIN))
						, portModeRegister(digitalPinToPort(LED_PIN))
						,digitalPinToBit(LED_PIN) );
*/
static constexpr auto led = StaticPin<LED_PIN>();
static constexpr auto tx = StaticPin<IR_PIN>();
HWSerialIO hwserial;
PrintStream cout;

void setup()
{
	led.SetOutput(); //pinMode(LED_PIN,OUTPUT);
	tx.SetOutput();
	hwserial.begin(9600);
	cout.begin(&hwserial);
	cout<<"Emitting at 38Khz with 500us intervals\n";
}

void loop()
{
	SCOPED_SIGNAL_PROCESSING; // allows use of xxxFast methods in AvrTLSignal.h inside current block
	
	for(int i=0;i<64;i++)
	{
		avrtl::setLinePWMFast<38000,avrtl::pwmval(0.5)>(tx,true,2000);
		tx = 0;
		avrtl::DelayMicrosecondsFast(2000);
	}
	avrtl::DelayMicrosecondsFast(200000);
}
