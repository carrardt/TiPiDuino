#include "AvrTL.h"
#include "AvrTLPin.h"
#include "AvrTLSignal.h"
#include "RFSniffer.h"
#include "RFSnifferProtocol.h"
#include "HWSerialIO.h"
#include "InputStream.h"

using namespace avrtl;

// pinout
#define RF_EMIT_PIN 2
#define LED_PIN 13

#define SERIAL_SPEED 9600

HWSerialIO hwserial; // pins 0,1
InputStream cin;
static auto rf_tx = StaticPin<RF_EMIT_PIN>();
static auto led = StaticPin<LED_PIN>();

RFSnifferProtocol protocol;

void setup()
{
	// setup pin mode
	rf_tx.SetOutput(); rf_tx=0;
	hwserial.begin(SERIAL_SPEED);
	cin.begin( &hwserial );
	
	protocol.init();
	protocol.bitSymbols[0] = 1500;
	protocol.bitSymbols[1] = 3000;
	protocol.messageBits = 8;
	protocol.latchSeqLen = 1;
	protocol.latchSeq[0] = 15000;
	protocol.nMessageRepeats = 4;
	protocol.latchGap = 10000;
	protocol.bitGap = 1000;
	protocol.coding = CODING_BINARY;
	protocol.flags = RFSnifferProtocol::RF_FLAG | RFSnifferProtocol::LOW_LEVEL_FLAG | RFSnifferProtocol::MODULATION_NONE ;
}

void loop(void)
{
	static uint8_t c=0;
	+c;
	/*char c;
	cin>>c;
	*/
	//blink(led);
	protocol.writeMessage((uint8_t*)&c,1, [](bool l,uint32_t t){ avrtl::setLineFlatFast(rf_tx,l,t); } );
	DelayMicroseconds(24000);
}
