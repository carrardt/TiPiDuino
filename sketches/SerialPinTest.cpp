#include <AvrTL.h>
#include <AvrTLPin.h>

#include "HWSerialNoInt/HWSerialNoInt.h"
#include "BasicIO/PrintStream.h"
#include "BasicIO/InputStream.h"

ByteStreamAdapter<HWSerialNoInt,100000UL> serialIO;
PrintStream cout;
InputStream cin;

void setup()
{
	cli();
	serialIO.setEndLine("\n\r");
	serialIO.m_rawIO.begin(57600);
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

