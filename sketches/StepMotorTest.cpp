#include <AvrTL.h>
#include <AvrTLPin.h>

#include "BasicIO/PrintStream.h"
#include "BasicIO/InputStream.h"

#include <avr/interrupt.h>

// #define USE_HW_SERIAL 1

#ifdef USE_HW_SERIAL
#include "HWSerialNoInt/HWSerialNoInt.h"
ByteStreamAdapter<HWSerialNoInt,100000UL> serialIO;
#else
#include <SoftSerial.h>
static auto rx = avrtl::StaticPin<0>();
static auto tx = avrtl::StaticPin<1>();
using SerialScheduler = TimeScheduler;
static auto rawSerialIO = make_softserial_hr<57600,SerialScheduler>(rx,tx);
static ByteStreamAdapter<decltype(rawSerialIO),100000UL> serialIO = { rawSerialIO };
#endif

PrintStream cout;
InputStream cin;

/* test with L293D and 4-wire step motor
 * ARDUINO 	L293D IN	L293D OUT	MOTOR		COIL
 * D2 		ENABLE1,2
 * D3 		ENABLE3,4
 * D4 		INPUT1		OUTPUT1		YELLOW		A-
 * D5 		INPUT2		OUTPUT2		RED			B-
 * D6		INPUT3		OUTPUT3		ORANGE		B+
 * D7		INPUT4		OUTPUT4		BLUE		A+
 */

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

/*
 *       6
 *    4     5
 *       7
 */

static void motor_cw(int32_t delay,int32_t inc)
{
	digitalWrite(4,HIGH);
	avrtl::DelayMicroseconds(delay); delay += inc;

	digitalWrite(7,LOW);
	avrtl::DelayMicroseconds(delay); delay += inc;

	digitalWrite(6,HIGH);
	avrtl::DelayMicroseconds(delay); delay += inc;

	digitalWrite(4,LOW);
	avrtl::DelayMicroseconds(delay); delay += inc;

	digitalWrite(5,HIGH);
	avrtl::DelayMicroseconds(delay); delay += inc;

	digitalWrite(6,LOW);
	avrtl::DelayMicroseconds(delay); delay += inc;

	digitalWrite(7,HIGH);
	avrtl::DelayMicroseconds(delay); delay += inc;

	digitalWrite(5,LOW);
	avrtl::DelayMicroseconds(delay);
}

static void motor_ccw(int32_t delay,int32_t inc)
{
	digitalWrite(4,HIGH);
	avrtl::DelayMicroseconds(delay);

	digitalWrite(6,LOW);
	avrtl::DelayMicroseconds(delay);

	digitalWrite(7,HIGH);
	avrtl::DelayMicroseconds(delay);

	digitalWrite(4,LOW);
	avrtl::DelayMicroseconds(delay);

	digitalWrite(5,HIGH);
	avrtl::DelayMicroseconds(delay);

	digitalWrite(7,LOW);
	avrtl::DelayMicroseconds(delay);

	digitalWrite(6,HIGH);
	avrtl::DelayMicroseconds(delay);

	digitalWrite(5,LOW);
	avrtl::DelayMicroseconds(delay);
}

void loop()
{
	int32_t count = 0;
	int32_t StartDelay = 10000;
	int32_t FastDelay = 5000;
	int32_t DelayInc = -100;

	// possible input : 500 15000 5000 -100
	cout<<"Count StartDelay FastDelay DelayInc ? ";
	cin>>count;
	cin>>StartDelay;
	cin>>FastDelay;
	cin>>DelayInc;
	cout<<endl;
	cout<<"Count="<<count<<", StartDelay="<<StartDelay<<", FastDelay="<<FastDelay<<", DelayInc="<<DelayInc<<endl;

	digitalWrite(2,HIGH);
	digitalWrite(3,HIGH);
	digitalWrite(4,LOW);
	digitalWrite(5,LOW);
	digitalWrite(6,LOW);
	digitalWrite(7,LOW);
	
	
	int32_t delay = StartDelay;
	if(count>0)
	{
		for(int32_t i=0;i<count;i++)
		{
			motor_cw(delay,DelayInc);
			if(delay>FastDelay) { delay += DelayInc*8; }
		}
	}
	else
	{
		for(int32_t i=0;i<(-count);i++)
		{
			motor_ccw(delay,DelayInc);
			if(delay>FastDelay) { delay += DelayInc*8; }
		}
	}

	digitalWrite(2,LOW);
	digitalWrite(3,LOW);
	digitalWrite(4,LOW);
	digitalWrite(5,LOW);
	digitalWrite(6,LOW);
	digitalWrite(7,LOW);
}

