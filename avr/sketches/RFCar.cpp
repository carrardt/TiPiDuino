//#define RFCAR_SERIAL_DEBUG 1
#define RFCAR_ATTINY_PINS 1


#include <AvrTL.h>
#include <AvrTLPin.h>
#include "RFSniffer/RFSnifferProtocol.h"
#include "TimeScheduler/TimeScheduler.h"

#ifdef RFCAR_SERIAL_DEBUG
#include "HWSerialNoInt/HWSerialNoInt.h"
#endif

#include <avr/interrupt.h>

using namespace avrtl;

#ifdef RFCAR_ATTINY_PINS
#define LEFT_MOTOR_PIN 	0
#define RIGHT_MOTOR_PIN 1
#define RF_RECEIVE_PIN 	2
#define RIGHT_SPEED_PIN 3
#define LEFT_SPEED_PIN 	4
#else
#define LEFT_MOTOR_PIN 	2
#define RIGHT_MOTOR_PIN 3
#define RF_RECEIVE_PIN 	8
#define RIGHT_SPEED_PIN 9
#define LEFT_SPEED_PIN 10
#define LED_PIN 	   13
#endif

#define TICKS_PER_ROUND 20

auto LeftWheel = StaticPin<LEFT_MOTOR_PIN>();
auto leftSpeed = StaticPin<LEFT_SPEED_PIN>();
auto RightWheel = StaticPin<RIGHT_MOTOR_PIN>();
auto rightSpeed = StaticPin<RIGHT_SPEED_PIN>();
auto rf_rx = StaticPin<RF_RECEIVE_PIN>();
RFSnifferProtocol remote;

#ifdef RFCAR_SERIAL_DEBUG
auto led = StaticPin<LED_PIN>();
ByteStreamAdapter<HWSerialNoInt,100000UL> serialIO;
PrintStream cout;
#else
auto led = NullPin();
#endif

//using Scheduler = TimeSchedulerT<AvrTimer1<> >;
static TimeScheduler ts;

void setup()
{
	// DI-O/Chacon RF433 protocol
	remote.coding = 77;
	remote.flags = 194;
	remote.latchSeqLen = 2;
	remote.latchSeq[0] = 10762;
	remote.latchSeq[1] = 2834;
	remote.latchGap = 221;
	remote.bitGap = 221;
	remote.bitSymbols[0] = 332;
	remote.bitSymbols[1] = 1397;
	remote.messageBits = 32;
	remote.nMessageRepeats = 4;
	
	// setup pin mode
	rf_rx.SetInput();
	leftSpeed.SetInput();
	rightSpeed.SetInput();
	LeftWheel.SetOutput(); 
	RightWheel.SetOutput(); 
	LeftWheel.Set(LOW);
	RightWheel.Set(LOW);
	
	cli();
	ts.start();
	
#	ifdef RFCAR_SERIAL_DEBUG
	serialIO.m_rawIO.begin(57600);
	cout.begin( &serialIO );
	cout<<"Ready"<<endl;
#	endif
}

static uint32_t turnWheels(int LCount, int RCount)
{
	ts.reset();
	bool ls = leftSpeed.Get();
	bool rs = rightSpeed.Get();
	int l=0, r=0;
	uint32_t c=0;
	LeftWheel.Set( l < LCount );
	RightWheel.Set( r < RCount );
	while( l < LCount || r < RCount )
	{
		ts.exec( 500, [&]()
			{
				bool nls = leftSpeed.Get();
				bool nrs = rightSpeed.Get();			
				if( nls != ls ) ++l;
				if( nrs != rs ) ++r;
				ls = nls;
				rs = nrs;
				LeftWheel.Set( l < LCount );
				RightWheel.Set( r < RCount );
				++c;
			} );
	}
	LeftWheel.Set(LOW);
	RightWheel.Set(LOW);
	
	if( r > l ) l = r;
	uint32_t t = (r>l) ? r : l;
	uint32_t ms = c/2;
	return (t*1000) / ms;
}

void loop()
{
	uint8_t message[8];
	led=false;
	int n = 0;
	do {
		n = remote.readMessage(rf_rx,message);
	} while( n != remote.messageBits );
	led=true;

	switch( message[3] )
	{
		case 144:
			turnWheels(128,128);
			break;
		case 128:
			turnWheels(0,128);
			break;
		case 145:
			turnWheels(128,0);
			break;
		case 129:	
			turnWheels(128,0);
			turnWheels(128,128);
			turnWheels(0,128);
			break;
		case 146: 
			turnWheels(0,128);
			turnWheels(128,128);
			turnWheels(128,0);
			break;
#		ifdef RFCAR_SERIAL_DEBUG
		case 130:
			uint32_t lspeed = turnWheels(256,0);
			uint32_t lsi = lspeed / TICKS_PER_ROUND;
			uint32_t lsf = ( (lspeed*100) / TICKS_PER_ROUND ) % 100;
			cout << "Left RPM = "<<lsi<<'.'<<lsf<<endl;

			uint32_t rspeed = turnWheels(0,256);
			uint32_t rsi = rspeed / TICKS_PER_ROUND;
			uint32_t rsf = ( (rspeed*100) / TICKS_PER_ROUND ) % 100;
			cout << "Right RPM = "<<rsi<<'.'<<rsf<<endl;
			break;
#		endif
	}
}

