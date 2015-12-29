#include <AvrTLPin.h>
#include <AvrTLSignal.h>
#include <AvrTL.h>
#include <PrintStream.h>
#include <InputStream.h>
#include <HWSerialIO.h>

using namespace avrtl;

// auto soft_rx = pin(10);
// auto soft_tx = pin(11);
auto led = StaticPin<13>();

HWSerialIO hwserial;
// auto softIO = make_softserial<19200>(soft_rx,soft_tx);

PrintStream cout;
InputStream cin;

void setup()
{
	led = false;
	
	//softIO.begin();
	hwserial.begin(9600);

	cout.begin( &hwserial );
	cin.begin( &hwserial );
	cout<<"Ready"<<endl;
}

void loop()
{
	int n = 0;
	cin >> n;
	cout << n << endl;
	blink(led);
	led = false;
}
