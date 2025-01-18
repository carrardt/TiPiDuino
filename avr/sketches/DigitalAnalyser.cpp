#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <AvrTL.h>
#include <PrintStream.h>
#include <HWSerialIO.h>

using namespace avrtl;

auto signalInput = StaticPin<2>();
auto led = StaticPin<13>();

HWSerialIO hwserial;
// auto softIO = make_softserial<19200>(soft_rx,soft_tx);

PrintStream cout;

void setup()
{
	signalInput.SetInput();
	led.SetOutput();
	led = false;
	
	//softIO.begin();
	hwserial.begin(57600);

	cout.begin( &hwserial );
	cout<<"Digital Analyser Ready"<<endl;
}

static bool lvl = false;
static uint16_t signalBuffer[512];

void loop()
{
	bool lvl = signalInput.Get();
	uint16_t nSamples = RecordSignal( signalInput, 1000000UL, 512, signalBuffer );
	if( nSamples>1 )
	{
		cout<<'['<<nSamples<<"] ";
		for(uint16_t i=0;i<nSamples;i++)
		{
			led = lvl;
			cout<< (lvl?'+':'-') << ticksToMicroseconds(signalBuffer[i])<<" ";
			lvl = !lvl;
		}
	}
	cout<<endl;
	led = false;
}
