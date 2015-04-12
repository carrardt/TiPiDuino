#include <AvrTL.h>
#include <PrintStream.h>
#include <InputStream.h>
#include <HWSerialIO.h>
//#include <SoftSerialIO.h>

using namespace avrtl;

// auto soft_rx = pin(10);
// auto soft_tx = pin(11);
auto led = pin(13);

HWSerialIO hwserial;
// auto softIO = make_softserial<19200>(soft_rx,soft_tx);

PrintStream cout;
InputStream cin;

void setup()
{
	led = false;
	
	//softIO.begin();
	hwserial.begin(19200);

	cout.begin( &hwserial );
	cin.begin( &hwserial );
	cout<<"Ready\n";
}

void loop()
{
	int n = 0;
	cin >> n;
	cout << n << '\n';
	blink(led);
	led = false;
}
