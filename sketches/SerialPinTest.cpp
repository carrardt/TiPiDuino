#include <AvrTL.h>
#include <AvrTLPin.h>

#include "BasicIO/PrintStream.h"
#include "BasicIO/InputStream.h"

#include <avr/interrupt.h>

// #define USE_HW_SERIAL 1
#define SERIAL_SPEED 19200

#ifdef __AVR_ATtiny84__
#define SERIAL_RX_PIN 9
#define SERIAL_TX_PIN 10
#else
#define SERIAL_RX_PIN 0
#define SERIAL_TX_PIN 1
#endif

#ifdef USE_HW_SERIAL
#include "HWSerialNoInt/HWSerialNoInt.h"
ByteStreamAdapter<HWSerialNoInt,100000UL> serialIO;
#else
#include <SoftSerial.h>
static auto rx = avrtl::StaticPin<SERIAL_RX_PIN>();
static auto tx = avrtl::StaticPin<SERIAL_TX_PIN>();
using SerialScheduler = TimeSchedulerT<avrtl::AvrTimer0NoPrescaler>;
static auto rawSerialIO = make_softserial_hr<SERIAL_SPEED,SerialScheduler>(rx,tx);
static ByteStreamAdapter<decltype(rawSerialIO),100000UL> serialIO = { rawSerialIO };
#endif

PrintStream cout;

void setup()
{
	cli();
#	ifdef USE_HW_SERIAL
		serialIO.m_rawIO.begin(SERIAL_SPEED);
#	else
		serialIO.m_rawIO.begin();
#	endif
	cout.begin( &serialIO );
	cout<<"Ready"<<endl;
}

#define BUF_LEN 64

void loop()
{
	int p = 0;
	bool value = false;
	char c = 0;
	int i=0;
	
	InputStream cin;

#	ifdef USE_HW_SERIAL
	cin.begin( &serialIO );
#	else
	uint8_t buffer[BUF_LEN];
	serialIO.m_rawIO.ts.start();
	do {
		c = serialIO.m_rawIO.readByteFast();   
		buffer[i++]=c;
	} while( c!='\n' && c!='\r' && i<(BUF_LEN-1) );
	serialIO.m_rawIO.ts.stop();
	cout<<"received "<<i<<" bytes"<<endl;
	buffer[i-1]='\0';
	cout<<"buffer='"<<((const char*)buffer)<<"'"<<endl;
	BufferStream bufStream(buffer,BUF_LEN-1);
	cin.begin(&bufStream);
#	endif

	c=' ';
	cin>>c;
	cin>>p;
	cout<<"command is "<<c<<","<<p<<endl;
	if(c=='r')
	{
		pinMode(p,INPUT);
		value = digitalRead(p);
		cout<<"pin #"<<p<<" -> "<<value<<endl;
	}
	else if(c=='w')
	{
		int x=0;
		cin>>x;
		value = x;
		cout<<"pin #"<<p<<" <- "<<value<<endl;
		pinMode(p,OUTPUT);
		digitalWrite(p,value);
	}
	else
	{
		cout<<"command must be r# or w# 0|1"<<endl;
	}
}

