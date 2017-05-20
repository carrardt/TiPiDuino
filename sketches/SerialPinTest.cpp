#include <AvrTL.h>
#include <AvrTLPin.h>

#include "BasicIO/PrintStream.h"
#include "BasicIO/InputStream.h"

// #define USE_HW_SERIAL 1

#ifdef USE_HW_SERIAL
#include "HWSerialNoInt/HWSerialNoInt.h"
ByteStreamAdapter<HWSerialNoInt,100000UL> serialIO;
#else
#include <SoftSerial.h>
static auto rx = avrtl::StaticPin<0>();
static auto tx = avrtl::StaticPin<1>();
using SerialScheduler = TimeSchedulerT<AvrTimer0NoPrescaler>;
static auto rawSerialIO = make_softserial_hr<57600,SerialScheduler>(rx,tx);
static ByteStreamAdapter<decltype(rawSerialIO),100000UL> serialIO = { rawSerialIO };
#endif

PrintStream cout;
InputStream cin;

void setup()
{
	cli();
	serialIO.setEndLine("\n\r");
#	ifdef USE_HW_SERIAL
		serialIO.m_rawIO.begin(57600);
#	else
		serialIO.m_rawIO.begin();
#	endif
	cout.begin( &serialIO );
	cin.begin( &serialIO );
	cout<<"Ready"<<endl;
}

void loop()
{
	int pin = 0;
	bool dir = false;
	bool value = 0;
	char c=' ';
	
	cin>>c;
	cin>>pin;
	if(c=='r')
	{
		value = digitalRead(pin);
		cout<<"pin #"<<pin<<" -> "<<value<<endl;
	}
	if(c=='w')
	{
		int x=0;
		cin>>x;
		value = x;
		cout<<"pin #"<<pin<<" <- "<<value<<endl;
		digitalWrite(pin,value);
	}
}

