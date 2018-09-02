#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTL/timer.h>
#include <BasicIO/PrintStream.h>
#include <HWSerial/HWSerialIO.h>
#include <SignalProcessing/SignalProcessing.h>

using namespace avrtl;

#define TRIGGER_PIN 3
#define ECHO_PIN 2

auto trigger = StaticPin<TRIGGER_PIN>();
auto echo = StaticPin<ECHO_PIN>();

HWSerialIO hwserial;
PrintStream cout;

void setup()
{
	trigger.SetOutput();
	echo.SetInput(); 
	hwserial.begin(9600);
	cout.begin( &hwserial );
	cout<<"Ready"<<endl;
}

void loop()
{
	uint16_t echo_gap = 0;
	uint32_t echo_length = 0;
	{
		SignalProcessing32 sp;
		trigger=true;
		avrtl::delayMicroseconds(10);
		trigger=false;
		echo_length = sp.PulseIn(echo,true,4000,&echo_gap);
		int32_t waitTime = 60000 - (echo_length+echo_gap);
		if(waitTime>0) { avrtl::delayMicroseconds(waitTime); }
	}
	cout<<"gap="<<echo_gap<<", len="<<echo_length<<endl;
}

