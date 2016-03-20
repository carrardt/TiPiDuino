#include <AvrTL.h>
#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include "RFSniffer/RFSnifferProtocol.h"

using namespace avrtl;

#define LEFT_MOTOR_PIN 2
#define RIGHT_MOTOR_PIN 3
#define RF_RECEIVE_PIN 9
#define LED_PIN 13

auto LeftWheel = StaticPin<LEFT_MOTOR_PIN>();
auto RightWheel = StaticPin<RIGHT_MOTOR_PIN>();
auto rf_rx = StaticPin<RF_RECEIVE_PIN>();
auto led = StaticPin<LED_PIN>();
RFSnifferProtocol remote;

static void stop()
{
	LeftWheel=false;
	RightWheel=false;
}

static void ahead()
{
	LeftWheel=true;
	RightWheel=true;
}

static void turnRight()
{
	LeftWheel=true;
	RightWheel=false;
}

static void turnLeft()
{
	LeftWheel=false;
	RightWheel=true;
}

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
	LeftWheel.SetOutput(); 
	RightWheel.SetOutput(); 
	stop();
}

static void goAhead()
{
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
			ahead();
			DelayMicroseconds(1500000UL);
			stop();
			break;
		case 128:
			for(int i=0;i<4;i++)
			{
				ahead();
				DelayMicroseconds(250000UL); 	
				turnRight();
				DelayMicroseconds(250000UL); 	
				ahead();
				DelayMicroseconds(250000UL); 	
				turnLeft();
				DelayMicroseconds(250000UL); 	
			}
			stop();
			break;
		case 145:
			turnRight();
			DelayMicroseconds(400000UL);
			stop();
			break;
		case 129:	
			turnRight();
			DelayMicroseconds(1000000UL); 	
			ahead();
			DelayMicroseconds(1500000UL); 	
			stop();
			break;
		case 146: 
			turnLeft(); 
			DelayMicroseconds(400000UL); 
			stop(); 
			break;
		case 130:
			turnLeft();
			DelayMicroseconds(1000000UL); 	
			ahead();
			DelayMicroseconds(1500000UL); 	
			stop();
			break;
	}
}

