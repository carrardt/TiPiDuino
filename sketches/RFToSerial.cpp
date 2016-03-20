#include "AvrTL.h"
#include "AvrTLPin.h"
#include "AvrTLSignal.h"
#include "BasicIO/HWSerialIO.h"
#include "BasicIO/PrintStream.h"
#include "RFSniffer/RFSnifferProtocol.h"

using namespace avrtl;

// pinout
#define RF_RECEIVE_PIN 9

#define SERIAL_SPEED 9600

HWSerialIO hwserial; // pins 0,1
PrintStream cout;
auto rf_rx = StaticPin<RF_RECEIVE_PIN>();
RFSnifferProtocol remote;

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
	hwserial.begin(SERIAL_SPEED);
	cout.begin( &hwserial );
	cout<<"Ready"<<endl;
}

void loop(void)
{
	uint8_t message[8];
	int n = 0;
	do {
		n = remote.readMessage(rf_rx,message);
	} while( n != remote.messageBits );
	for(int i=0;i<4;i++) cout<<(int)message[i]<<' ';
	cout<<endl;
}
