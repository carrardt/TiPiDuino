#include "AvrTL.h"
#include "AvrTLPin.h"
#include "AvrTLSignal.h"
#include "HWSerialIO.h"
#include "PrintStream.h"

using namespace avrtl;

// pinout
#define RF_RECEIVE_PIN 8
#define LED_PIN 13

#define SERIAL_SPEED 9600

HWSerialIO hwserial; // pins 0,1
PrintStream cout;
static auto rf_rx = StaticPin<RF_RECEIVE_PIN>();
static auto led = StaticPin<LED_PIN>();

void setup()
{
	// setup pin mode
	rf_rx.SetInput();
	hwserial.begin(SERIAL_SPEED);
	cout.begin( &hwserial );
}

void loop(void)
{
	uint16_t buf[8];
	{
		SCOPED_SIGNAL_PROCESSING;
		long p = 0;
		do {
			p = avrtl::PulseInFast(rf_rx,false,30000);
		} while( p<13000 | p>17000 );
		for(int i=0;i<8;i++) {  buf[i]= avrtl::PulseInFast(rf_rx,false,5000); }
	}
	blink(led);
	for(int i=0;i<8;i++) { cout << buf[i]<<' '; }
	cout<<endl;
}
